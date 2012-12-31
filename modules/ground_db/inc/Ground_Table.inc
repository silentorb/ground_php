<?php

class Ground_Table {

  public $name;
  public $fields;
  public $ground;

  public static $types = array(
      'INT' => 'int',
      'TINYINT' => 'bool',
      'SMALLINT' => 'int',
      'VARCHAR' => 'string',
      'TEXT' => 'text',
      'LONGTEXT' => 'text',
  );
  
  function __construct($name, $ground) {
    $this->name = $name;
    $this->ground = $ground;
    $this->load_fields();
  }
  
  public static function convert_to_property_type($field_type) {
    $name = preg_replace('/\s*\(.*\).*/', '', $field_type);
    $name = strtoupper($name);
    return Ground_Table::$types[$name];
  }
  
  public static function create_from_trellis($trellis, $ground = null) {
    if (!$ground)
      $ground = $trellis->ground;

    $table = new Ground_Table($trellis->get_plural(), $ground);
    $table->trellis = $trellis;
    return $table;
  }

  function load_fields() {
    $fields = $this->ground->db->query_objects('SHOW COLUMNS FROM ' . $this->name);
    $this->fields = array();
    foreach ($fields as $item) {
      $field = new stdClass();
      $field->name = $item->Field;
      $field->type = $item->Type;
      $field->allow_null = $item->Null;
      $field->default = $item->Default;
      $this->fields[$field->name] = $field;
    }
  }

  function get_vineyard_layer() {
    $layer = new stdClass();
    $layer->properties = array();
    foreach ($this->fields as $item) {
      $property = new stdClass();
      $property->type = Ground_Table::convert_to_property_type($item->type);
      $layer->properties[$item->name] = $property;
    }
    return $layer;
  }

  function get_ground_layer() {
    
  }

}
