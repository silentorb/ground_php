<?php

class Table {

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
  }

  public static function convert_to_property_type($field_type) {
    $name = preg_replace('/\s*\(.*\).*/', '', $field_type);
    $name = strtoupper($name);
    return Table::$types[$name];
  }

  public static function create_from_trellis($trellis, $ground = null) {
    if (!$ground)
      $ground = $trellis->ground;

    $table = new Table($trellis->get_plural(), $ground);
    $table->trellis = $trellis;
    $table->load_from_database();
    return $table;
  }

  function load_fields() {
    $rows = $this->ground->db->query_objects('SHOW COLUMNS FROM ' . $this->name);
    $result = array();
    foreach ($rows as $row) {
      $field = new stdClass();
      $field->name = $row->Field;
      $field->type = $row->Type;
      $field->allow_null = $row->Null;
      $field->default = $row->Default;
      $result[$field->name] = $field;
    }

    return $result;
  }

  function load_from_database() {
    $this->fields = $this->load_fields();
    $this->indexes = $this->load_indexes();
  }

  function load_from_schema($source) {
    if (isset($this->ground->trellises[$this->name])) {
      // Create bi-connection.  For now I don't know of any case where
      // tables and trellises would be dynamically modified, so I'm not 
      // using MetaHub connections for this.
      $this->trellis = $this->ground->trellises[$this->name];
      $this->trellis->table = $this;
      $this->name = $this->trellis->get_plural();
    }
    
    if (isset($source->table_name)) {
      $this->name = $source->table_name;
    }
  }

  function load_indexes() {
    $rows = $this->ground->db->query_objects('SHOW INDEX FROM ' . $this->name);
    $result = array();
    foreach ($rows as $row) {
      $name = $index->Key_name;

      // Multiple rows from this query can be part of the same query, so combine them if they are.
      if (!isset($result[$name])) {
        $index = new stdClass();
        $result[$name] = $index;
        $index->name = $name;
        $index->fields = array();
        $index->unique = !$row->Non_unique;
        $index->comment = $row->Comment;
        $index->index_comment = $row->Index_comment;
        $index->collation = $row->Collation;
        $index->cardinality = $row->Cardinality;
        $index->index_type = $row->Index_Type;
      }
      else {
        $index = $result[$name];
      }

      $index->fields[] = $row->Column_name;
    }

    return $result;
  }

  function get_vineyard_layer() {
    $layer = new stdClass();
    $layer->properties = array();
    foreach ($this->fields as $item) {
      $property = new stdClass();
      $property->type = Table::convert_to_property_type($item->type);
      $layer->properties[$item->name] = $property;
    }
    return $layer;
  }

  function get_ground_layer() {
    
  }

}

