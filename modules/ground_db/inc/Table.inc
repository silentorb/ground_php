<?php

class Table {

  public $name;
  public $properties = array();
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
    // If a developer wants to replace an existing table object,
    // this function is not the way to do it.
    if ($trellis->table)
      return $trellis->table;
    
    if (!$ground)
      $ground = $trellis->ground;

    $table = new Table($trellis->get_table_name(), $ground);
    $table->trellis = $trellis;
    return $table;
  }

  public function generate_create_sql() {
    $trellis = $this->trellis;
    $keys = array();
    $fields = array();
    
    foreach ($trellis->properties as $property) {
      $type = $property->get_field_type();
      if ($type === null)
        continue;

      $field_name = $property->get_field_name();
      $field_sql = "`$field_name` $type";
      if ($field_name == $trellis->primary_key) {
        $field_sql .= ' AUTO_INCREMENT';
        $keys[] = "PRIMARY KEY (`$field_name`)\n";
      }

      $fields[] = $field_sql;
    }

    // Currently can't create a table without fields.  Seems reasonable enough.
    if (count($fields) == 0)
      return;

    $fields = array_merge($fields, $keys);
    $sql = 'CREATE TABLE IF NOT EXISTS `' . $this->name . "` (\n";
    $sql .= implode(",\n", $fields) . "\n";

    $sql .= ");\n";
    return $sql;
  }

  static function load_fields($db, $name) {
    $rows = $db->query_objects('SHOW COLUMNS FROM ' . $name);
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
    $this->properties = Table::load_fields($this->ground->db, $this->name);
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

    MetaHub::extend($this, $source);
//    if (isset($source->name)) {
//      $this->name = $source->name;
//    }
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
    foreach ($this->properties as $item) {
      $property = new stdClass();
      $property->type = Table::convert_to_property_type($item->type);
      $layer->properties[$item->name] = $property;
    }
    return $layer;
  }

  function get_ground_layer() {
    
  }
  
/* Example:
   Table::get_vineyard_json('dbname', 'table_name');
*/
  
  static function get_vineyard_json($database_name, $table_name) {
    $ground = new Ground($database_name);
    $table = new Table($table_name, $ground);
    $table->load_from_database();
    $data[$table_name] = $table->get_vineyard_layer();
    return json_encode($data);
  }

}

