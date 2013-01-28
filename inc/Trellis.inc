<?php

class Bloom_Property {

  public $name;
  public $parent;
  public $type;
  public $trellis;
  public $readonly = false;
  public $insert_trellis;
  public $link_class = null;
  public $property = null; // String reference to the property this connects to on the other trellis.

  public function __construct($name, $source, $parent) {
    $names = get_object_vars($source);
    foreach ($names as $key => $item) {
      if (property_exists($this, $key)) {
        $this->{$key} = $source->{$key};
      }
    }

    $this->name = $name;
    $this->field_name = $name;
//
//    if ($this->type == 'reference') {
//      $this->field_name = $name . '_' . $parent->primary_key;
//    }
//    else {
//    }
    $this->parent = $parent;
  }

  public function get_data() {
    $result = new stdClass();
//    $result->name = $this->name;
    $result->type = $this->type;
    if ($this->trellis)
      $result->trellis = $this->trellis;

    if ($this->readonly)
      $result->readonly = $this->readonly;

    if ($this->insert_trellis)
      $result->insert_trellis = $this->insert_trellis;

    return $result;
  }

  public function get_field_type() {
    $property_type = $this->get_property_type();
    if ($property_type) {
      return $property_type->field_type;
    }

    return null;
  }

  public function get_default() {
    $type = $this->get_property_type();
    return $type->default;
  }

  public function get_link_class() {
    if ($field = $this->get_field_override()) {
      if ($field->link_class)
        return $field->link_class;
    }

    return 'Link_Trellis';
  }

  public function get_field_override($create_if_missing = false) {
    $table = $this->parent->table;
    if (!$table) {
      if (!$create_if_missing)
        return null;

      $table = Table::create_from_trellis($this->parent);
    }

    if (!array_key_exists($this->name, $table->properties)) {
      if (!$create_if_missing)
        return null;

      $table->properties[$this->name] = new stdClass();
    }

    return $table->properties[$this->name];
  }

  public function get_other_property() {
    $other_table = $this->parent->ground->trellises[$this->trellis];

    if ($this->property) {
      return $other_table->properties[$this->property];
    }
    else {
      foreach ($other_table->properties as $property) {
        if ($property->trellis == $this->parent->name) {
          return $property;
        }
      }
    }

    return null;
  }

  public function get_property_type() {
    $types = $this->parent->ground->property_types;
    if (array_key_exists($this->type, $types)) {
      return $types[$this->type];
    }

    return null;
  }

  public function get_field_name() {
    if ($field = $this->get_field_override()) {
      if (isset($field->name)) {
        return $field->name;
      }
    }

    return $this->name;
  }

  public function get_referenced_trellis() {
    $other = $this->parent->ground->trellises[$this->trellis];
    if (!$other)
      throw new Exception('Could not find reference to property ' . $this->name . ' for ' . $this->trellis . '.');
    return $other;
  }

  public function override_field($attribute, $value) {
    $field = $this->get_field_override(true);
    $field->{$attribute} = $value;
  }

  public function query() {
    return $this->parent->get_table_name() . '.' . $this->get_field_name();
  }

}

class Trellis {

  public $plural;
  public $parent;
  public $ground;
  public $table;
  public $primary_key = 'id';
  // Properties that are specific to this trellis and not inherited from a parent trellis
  public $properties = array();
  // Every property including inherited properties
  public $all_properties = array();
  public $links = array();

  public function __construct($name, $ground) {
    $this->ground = $ground;
    $this->name = $name;
//    $this->update_core_properties();
  }

  public function add_property($name, $source) {
    if (is_array($source))
      $source = (object) $source;

    $property = new Bloom_Property($name, $source, $this);
    $this->properties[$name] = $property;
    $this->all_properties[$name] = $property;
    if ($property->type != 'list') {
      $this->core_properties[] = $property;
    }
    if ($property->type == 'reference' || $property->type == 'list') {
      $this->links[] = $property;
    }

    return $property;
  }

  public function check_primary_key() {
    if (!$this->properties[$this->primary_key] && $this->parent) {
      $property = $this->parent->properties[$this->parent->primary_key];
      $this->properties[$this->primary_key] = new Bloom_Property($this->primary_key, $property, $this);
    }
  }

  public function clone_property($property_name, $target_trellis) {
    if (!array_key_exists($property_name, $this->properties))
      throw new Exception("$this->name does not have a property named $property_name.");

    $target_trellis->add_property($property_name, $this->properties[$property_name]);
  }

  public function create_object() {
    $result = new stdClass();
    foreach ($this->properties as $name => $property) {
      // Primary keys should be null, not the default value.
      // This informs both ground and any SQL inserts that the primary key
      // is not set and still needs to be assigned a value.
      if ($name == $this->primary_key) {
        $result->{$name} = null;
      }
      else {
        $result->{$name} = $property->get_default();
      }
    }
    return $result;
  }

  public function get_all_properties() {
    $result = array();
    $tree = $this->get_tree();
    foreach ($tree as $trellis) {
      foreach ($trellis->properties as $property) {
        $result[$property->name] = $property;
      }
    }

    return $result;
  }

  public function get_core_properties() {
    $result = array();
    foreach ($this->properties as $property) {
      if ($property->type != 'list') {
        $result[] = $property;
      }
    }

    return $result;
  }

  public function get_data() {
    $result = new stdClass();
//    $result->name = $this->name;
    if ($this->plural)
      $result->plural = $this->plural;

    if ($this->parent)
      $result->parent = $this->parent->name;

    if ($this->primary_key != 'id')
      $result->primary_key = $this->primary_key;

    $result->properties = array();
    foreach ($this->properties as $property) {
      $result->properties[$property->name] = $property->get_data();
    }
    return $result;
  }

  public function get_link_property($other_table) {
    foreach ($this->links as $link) {
      if ($link->trellis == $other_table->name) {
        return $link;
      }
    }

    return null;
  }

  public function get_links() {
    $result = array();
    foreach ($this->properties as $property) {
      if ($property->trellis)
        $result[] = $property;
    }
    return $result;
  }

  public function get_object_id($object) {
    return $object->{$this->primary_key};
  }

  public function get_plural() {
    if ($this->plural)
      return $this->plural;

    return $this->name . 's';
  }

  public function get_primary_property() {
    return $this->all_properties[$this->primary_key];
  }

  public function get_table_name() {
    if ($this->table)
      return $this->table->name;

    if ($this->plural)
      return $this->plural;

    return $this->name . 's';
  }

  public function get_tree() {
    $trellis = $this;
    $tree = array();

    do {
      array_unshift($tree, $trellis);
    }
    while ($trellis = $trellis->parent);

    return $tree;
  }

  function is_a($name) {
    $trellis = $this;
    do {
      if ($trellis->name == $name)
        return true;
    }
    while ($trellis = $trellis->parent);
    return false;
  }

//  public function load_from_database($table_name) {
//    
//  }

  public function load_from_object($source) {
    $names = get_object_vars($source);

    foreach ($names as $name => $item) {
      if ($name != 'name' && $name != 'properties' && property_exists($this, $name) && $source->{$name} !== null) {
        $this->{$name} = $source->{$name};
      }
    }

    foreach ($source->properties as $key => $property) {
      $this->add_property($key, $property);
    }
  }

  public function parent_query($query) {
    if ($this->parent) {
      $parent = $this->parent;
      $parent_table = $parent->get_table_name();
      $query->add_join('JOIN ' . $parent->get_table_name() .
              ' ON ' . $parent_table . '.' . $parent->primary_key .
              ' = ' . $query->main_table . '.' . $this->primary_key);

      $query->add_source($parent, false);
//      $parent->parent_query($query);
    }
  }

  public function query_primary_key() {
    return $this->get_table_name() . '.' . $this->primary_key;
  }

  public function query_property($property) {
    return $this->get_table_name() . '.' . $property;
  }

  public function set_parent($parent) {
    $this->parent = $parent;
    $this->all_properties = array();

    foreach ($parent->all_properties as $name => $property) {
      $this->all_properties[$name] = $property;
    }

    if (!$parent->primary_key)
      throw new Exception("$parent->name needs a primary key when being inherited by $this->name.");

    $parent->clone_property($parent->primary_key, $this);

    $this->primary_key = $parent->primary_key;
  }

  /*
    public function update_core_properties() {
    $this->core_properties = array();
    foreach ($this->properties as $property) {
    if ($property->type != 'list') {
    $this->core_properties[] = $property;
    }
    if ($property->type == 'reference' || $property->type == 'list') {
    $this->links[] = $property;
    //        $property->other = $this->ground->trellises[$property->trellis];
    }
    }

    if ($this->parent) {
    $this->all_properties = array_merge($this->parent->all_properties, $this->properties);
    }
    else {
    $this->all_properties = $this->properties;
    }
    }
   */
}
