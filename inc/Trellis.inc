<?php

define("RELATIONSHIP_ONE_TO_ONE", 1);
define("RELATIONSHIP_ONE_TO_MANY", 2);
define("RELATIONSHIP_MANY_TO_MANY", 3);

class Bloom_Property {

  public $name;
  public $parent;
  public $type;
  public $trellis;
  public $readonly = false;
  // This is going to be deprecated in favor of $insert
  public $insert_trellis;
  public $insert;
  public $link_class = null;
  public $property = null; // String reference to the property this connects to on the other trellis.
  public $default;

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
      return $property_type->get_field_type();
    }

    throw new Exception("$this->name could not find valid field type.");
  }

  public function get_field_value($value) {
    if (is_string($value))
      $value = str_replace("'", "\\'", $value);

    if ($value === true)
      return 'TRUE';
    else if ($value === false)
      return 'FALSE';

    if ($this->type == 'string' || $this->type == 'text') {
      $value = "'" . preg_replace("/[\r\n]+/", '\n', $value) . "'";
    }
    else if ($this->type == 'reference') {
      if (is_array($value)) {
        $value = (object) $value;
      }
      if (is_object($value)) {
        if ($value->trellis) {
          $trellis = $value->trellis;
        }
        else {
          $trellis = $this->trellis;
        }
        $ground = $this->parent->ground;
        $trellis = $ground->trellises[$trellis];

        $object = $ground->update_object($trellis, $value);
        $other_id = $this->get_other_id($value);
        if ($other_id !== null) {
          $value = $other_id;
        }
        else {
          $value = $object->{$trellis->primary_key};
        }
      }
    }

    if ($value === null) {
      $value = 'NULL';
    }

    if (is_object($value)) {
      throw new Exception("Property $this->name cannot be an object.");
    }

    if (is_array($value))
      throw new Exception("Property $this->name cannot be an array.");

    return $value;
  }

  function get_filter($value) {
    if ($value === 'null' && $this->type != 'string') {
      return $this->query() . ' IS NULL';
    }
    else {
      return $this->query() . ' = ' . $this->get_field_value($value);
    }
  }

  public function get_default() {
    if (isset($this->default))
      return $this->default;

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

  public function get_other_property($create_if_none = true) {
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

    if ($this->trellis == $this->parent->name)
      return null;

    if (!$create_if_none)
      return null;

    // If there is no existing connection defined in this trellis, create a dummy
    // connection and assume that it is a list.  This means that implicit connections
    // are either one-to-many or many-to-many, never one-to-one.
    $attributes = new stdClass();
    $attributes->type = "list";
    $attributes->trellis = $this->parent->name;
    $property = new Bloom_Property($other_table->name, $attributes, $other_table);
    return $property;
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

      if (isset($field->share)) {
        return $field->share;
      }
    }

    return $this->name;
  }

  public function get_referenced_trellis() {
    $other = $this->parent->ground->trellises[$this->trellis];
    if (!$other) {
      throw new Exception('Could not find reference to property ' . $this->name . ' for ' . $this->trellis . '.');
    }
    return $other;
  }

  public function get_relationship() {

    // First see if the table field tells us the relationship.
    $field = $this->get_field_override();
    if ($field && isset($field->relationship)) {
      switch ($field->relationship) {
        case 'one-to-one':
          return RELATIONSHIP_ONE_TO_ONE;
        case 'one-to-many':
          return RELATIONSHIP_ONE_TO_MANY;
        case 'many-to-many':
          return RELATIONSHIP_MANY_TO_MANY;
      }
    }

    // Otherwise, infer it from the property on the other side of the connection.
    $other_property = $this->get_other_property();
    if ($other_property === null)
      throw new Exception($this->parent->name . '->' . $this->name . ' does not have a reciprocal reference on ' . $link->other->name . '.');

    if ($this->type == 'list' && $other_property->type == 'list') {
      return RELATIONSHIP_MANY_TO_MANY;
    }
    else if ($this->type == 'list') {
      return RELATIONSHIP_ONE_TO_MANY;
    }

    return RELATIONSHIP_ONE_TO_ONE;
  }

  public function get_other_id($object) {
    $trellis = $this->get_referenced_trellis();
    if (isset($object->{$trellis->primary_key})) {
      return $object->{$trellis->primary_key};
    }

//    $property_name = $this->parent->name . '->' . $this->name;
//    throw new Exception("Cannot update reference because value for '$property_name' is an object that does not have an id.");
    return null;
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

  // It's a lot cleaner and safer to just query the links instead of track them.
//  public $links = array();

  public function __construct($name, $ground) {
    $this->ground = $ground;
    $this->name = $name;
//    $this->update_core_properties();
  }

  public function add_property($name, $source) {
    if (!$name) {
      throw new Exception("Attempt to add a property with no name to trellis $this->name.");
    }

    if (is_array($source))
      $source = (object) $source;

    $property = new Bloom_Property($name, $source, $this);
    $this->properties[$name] = $property;
    $this->all_properties[$name] = $property;
//    if ($property->type != 'list') {
//      $this->core_properties[] = $property;
//    }
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
        if (!$name) {
          throw new Exception();
        }
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

  function get_authorized_properties($account = null, $fortress = null, $access = 1) {
    if ($fortress === null)
      return $this->get_all_properties();

    $properties = $this->get_all_properties();
    $base_path = $this->name . '/';
    $result = array();
    foreach ($properties as $property) {
      $property_access = $fortress->access($account, $base_path . $property->name);
      if ($property >= $access) {
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

  public function get_id($value) {
    if (is_object($value)) {
      return $value->{$this->primary_key};
    }
    else if (is_array($value)) {
      return $value[$this->primary_key];
    }

    return $value;
  }

  public function get_link_property($other_table) {
    foreach ($this->all_properties as $link) {
      if ($link->trellis == $other_table->name) {
        return $link;
      }
    }

    // If there is no existing connection defined in this trellis, create a dummy
    // connection and assume that it is a list.  This means that implicit connections
    // are either one-to-many or many-to-many, never one-to-one.
    $attributes = new stdClass();
    $attributes->type = "list";
    $attributes->trellis = $other_table->name;
    $property = new Bloom_Property($other_table->name, $attributes, $this);
    return $property;
  }

  public function get_all_links() {
    $result = array();
    $properties = $this->get_all_properties();
    foreach ($properties as $property) {
      if ($property->trellis)
        $result[$property->name] = $property;
    }
    return $result;
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
    if ($this->table) {
      if ($this->table->db_name)
        return $this->table->db_name . '.' . $this->table->name;
      else
        return $this->table->name;
    }
    if ($this->plural)
      return $this->plural;

    return $this->name . 's';
  }

  public function get_table_query() {
    if ($this->table && $this->table->query)
      return $this->table->query;

    return $this->get_table_name();
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
      $query->add_join('JOIN ' . $parent->get_table_query() .
          ' ON ' . $parent_table . '.' . $parent->primary_key .
          ' = ' . $query->main_table . '.' . $this->primary_key);

      $query->add_source($parent, false);
//      $parent->parent_query($query);
    }
  }

  function get_join($main_table) {
    $result = '';
    if ($this->parent) {
      $parent = $this->parent;
      $parent_table = $parent->get_table_name();

      $result = 'JOIN ' . $parent->get_table_query() .
          ' ON ' . $parent_table . '.' . $parent->primary_key .
          ' = ' . $main_table . '.' . $this->primary_key;
    }

    return $result;
  }

  public function query_primary_key() {
    return $this->get_table_name() . '.' . $this->primary_key;
  }

  public function query_property($property) {
    return $this->get_table_name() . '.' . $property;
  }

  public function remove_property($property) {
    if (is_string($property)) {
      $name = $property;
      $property = $this->properties[$name];
    }
    else {
      $name = $property->name;
    }
    unset($this->properties[$name]);
    unset($this->all_properties[$name]);

    $index = array_search($status, $quest->links);
    if ($index > -1)
      unset($quest->links[$index]);
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
