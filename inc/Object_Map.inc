<?php

class Object_Field {

  public $name;
  public $type;
  public $parent;
  public $alias = null;
  public $render = true;

  public function __construct($name, $type, $parent, $alias = null) {
    $this->name = $name;
    $this->type = $type;
    $this->parent = $parent;
    $this->alias = $alias;
  }

  function get_alias() {
    if ($this->parent->is_root) {
      if ($this->alias)
        return $this->alias;

      return $this->name;
    }
    else if ($this->alias)
      return $this->parent->name . '_' . $this->alias;
    else
      return $this->parent->name . '_' . $this->name;
  }
}

class Object_Map {

  public $fields = array();
  public $name = '';
  public $table_name = '';
  public $is_array = false;
  public $aliases = array();
  public $location_added = false;
  public $location_join_added = false;
  public $is_root = false;

  public function __construct($name, $fields, $is_root = false) {
    $this->name = $name;
    $this->table_name = $name;
    $this->add_fields($fields);
    $this->is_root = $is_root;
  }

  public function add_field($name, $type, $alias = null) {
    $field = new Object_Field($name, $type, $this, $alias);
    $this->fields[$name] = $field;
    return $field;
  }

  function add_fields($new_fields, $render = true) {
    foreach ($new_fields as $name => $type) {
      $field = $this->add_field($name, $type);
      $field->render = $render;
    }
//    $this->fields = array_merge($this->fields, $new_fields);
  }

  function render_fields() {
    $sql = '';
    $x = 0;
    foreach ($this->fields as $field) {
      if (!$field->render)
        continue;

      if ($x++ > 0) {
        $sql .= ', ';
      }
      // Prefix the table name to the field name to reduce ambiguity.  The prefix will be removed at translation
      $sql .= " $this->table_name." . $field->name . ' AS ' . $field->get_alias();
    }

    if ($x == 0)
      return false;
    
    return $sql;
  }

  // Massages the data returned from the database.
  function translate($source) {
    if ($this->is_root) {
      $this->translate_self($source);
      return null;
    }

    $result = new stdClass();

    foreach ($this->fields as $name => $field) {
      $full_name = $field->get_alias();

      if (property_exists($source, $full_name)) {
        $result->$name = $this->convert_value($source->$full_name, $field->type);
        unset($source->$full_name);
      }
      else {
        $result->$name = null;
      }
    }

    reset($this->fields); //Safety - sets pointer to top of array
    $full_name = key($this->fields);
    if ($result->$full_name === null) {
      return null;
    }

    if ($item->is_array)
      $source->{$this->name}[] = $result;
    else
      $source->{$this->name} = $result;

    return $result;
  }

  protected function convert_value($value, $type) {
    if ($value == null) {
      if ($type == 'bool')
        return false;

      return null;
    }
    switch ($type) {
      case 'int':
        return (int) $value;
        break;
      case 'string':
        return $value;
        break;
      case 'bool':
        return str_to_bool($value);
        break;
      case 'float':
        return (float) $value;
        break;
    }
  }

  function translate_self($source) {
    foreach ($this->fields as $name =>$field) {
      if (property_exists($source, $name)) {
        $source->$name = $this->convert_value($source->$name, $field->type);
      }
    }

    return $source;
  }

}
