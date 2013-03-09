<?php

class CCK_Trellis {

  public $ground;
  public $trellis;
  public $table;
  public $field_types;

  public function __construct($ground, $name, $field_types, $prefix) {
    $this->ground = $ground;
    $this->field_types = $field_types;

    // Create Trellis
//    $this->trellis = new Trellis($name, $ground);
    $this->trellis = $ground->add_trellis($name, null, false);

    // Create Table
    $this->table = new Table($prefix . $name, $ground);
    $this->table->connect_trellis($this->trellis);
    $ground->tables[$name] = $this->table;
  }

  function add_join_property($info, $field) {
    $source = new stdClass();
    $source->type = 'list';
//    $source->trellis = 'content_type_' . $field->name;
    $source->trellis = $field->name;
    $other_trellis = $this->ground->trellises[$field->name];
//    if ($other_trellis) {
//      $attributes = new stdClass();
//      $attributes->type = 'reference';
//      $attributes->trellis = $this->trellis->name;
//      $other_trellis->add_property($this->trellis->name, $attributes);
//      $other_table = $this->ground->tables[$field->name];
//      $table_field = new stdClass();
//      $table_field->share = 'nid';
//      $other_table->properties[$this->trellis->name] = $table_field;
//    }
//    if (!$ground->trellises[$source->trellis])
//      $this->create_link_trellis($field);

    $property = new Bloom_Property($field->name, $source, $this->trellis);
    $this->trellis->add_property($field->name, $property);

    $table_property = new stdClass();
    $table_property->name = $field->name . '_' . $info->suffix;
    $table_property->relationship = 'one-to-many';
    $this->table->properties[$field->name] = $table_property;
  }

  function add_property($info, $field, $name, $suffix = '') {
    $source = new stdClass();
    $source->type = $info->type;
    if ($info->trellis)
      $source->trellis = $info->trellis;

    if ($field->multiple)
      $source->type = 'list';

    $property = new Bloom_Property($name, $source, $this->trellis);
    $this->trellis->add_property($name, $property);
    $table_property = new stdClass();
    $table_property->name = 'field_' . $name;
    if ($suffix)
      $table_property->name .= '_' . $suffix;

    $this->table->properties[$name] = $table_property;
  }

  function add_properties($info, $field) {
    if (is_array($info->properties)) {
      foreach ($info->properties as $property) {
        $this->add_property($property, $field, $field->name . '_' . $property->suffix);
      }
    }
    else {
      $this->add_property($info, $field, $field->name, $info->suffix);
    }
  }

//  function create_link_trellis($field) {
//    $new_field = new stdClass();
//    $new_field->name = $field->name;
//    $new_field->type = $field->type;
//    $cck_trellis = new CCK_Trellis($this->ground, $field->name, $this->field_types);
//    $attributes = new stdClass();
//    $attributes->type = 'int';
//    $cck_trellis->add_property('nid', $attributes);
//    $cck_trellis->add_property('vid', $attributes);
//    $cck_trellis->add_property('delta', $attributes);
//    $cck_trellis->primary_key = 'nid'; // This is not accurate but practical.
//    $cck_trellis->create_properties(array($new_field));
//  }

  function create_properties($fields, $cross_table = false) {
    // Create Properties
    foreach ($fields as $field) {
      $info = $this->field_types[$field->type];
      if (!$info) {
        throw new Exception("No CCK field info for $field->type.");
      }

      if (!$field->db_storage && !$cross_table) {
        $this->add_join_property($info, $field);
      }
      else {
        $this->add_properties($info, $field);
      }
    }
  }

}