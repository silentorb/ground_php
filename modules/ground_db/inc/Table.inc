<?php

class Table {

  public $name;
  public $properties = array();
  public $indexes = array();
  public $ground;
  public $db_name;
  public $query;
  // Array: An override for Vineyard's single key design.  $primary_keys
  // mostly just effects table creation.
  public $primary_keys = null;
  public static $types = array(
      'CHAR' => 'string',
      'DECIMAL' => 'decimal',
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

  public function connect_trellis($trellis) {
    $this->trellis = $trellis;
    $trellis->table = $this;
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
    $table->connect_trellis($trellis);
    return $table;
  }

  public static function create_from_join($join, $ground = null) {
    if ($join->table)
      return $join->table;

    if (!$ground)
      $ground = $join->ground;

    $table = new Table($join->table_name, $ground);
    return $table;
  }

  public function create_sql() {
    $fields = array();

    foreach ($this->properties as $name => $property) {
      $type = $this->ground->property_types[$property->type];
      $field = array(
          'name' => $name,
          'type' => $type->get_field_type(),
      );
      if (isset($property->default))
        $field['default'] = $property->default;

      $fields[] = $field;
    }

    if (is_array($this->primary_keys) && count($this->primary_keys) > 0)
      $primary_keys = $this->primary_keys;
    else
      $primary_keys = array($this->primary_key);

    return Table::create_sql_from_array($this->name, $fields, $primary_keys);
  }

  public static function create_sql_from_array($table_name, $source, $primary_keys = array(), $indexes = array()) {
    $fields = array();

    foreach ($source as $index => $field) {
      if (isset($field['name']))
        $name = $field['name'];
      else
        $name = $index;

      $type = $field['type'];

      if ($type === null)
        continue;

      $field_sql = "`$name` $type";
      if (in_array($name, $primary_keys)) {
        if (strstr($type, 'INT') && $primary_keys[0] == $name) {
          $field_sql .= ' AUTO_INCREMENT';
        }
      }
      if (isset($field['default'])) {
        $field_sql .= ' DEFAULT ' . Table::format_value($field['default']);
      }
      $fields[] = $field_sql;
    }

    // Currently can't create a table without fields.  Seems reasonable enough.
    if (count($fields) == 0) {
      if (count($source) > 0)
        throw new Exception("None of the field arguments for creating $table_name have a type.");
      else
        throw new Exception("Cannot create a table without fields: $table_name.");
    }

    $primary_fields = array();
    foreach ($primary_keys as $key) {
      $primary_fields[] = "`$key`";
    }
    $fields[] = "PRIMARY KEY (" . implode(', ', $primary_fields) . ")\n";
    foreach ($indexes as $index) {
      $index = (object) $index;
      if ($index->unique) {
        $index_fields = implode(', ', $index->fields);
        $fields[] = 'UNIQUE KEY ' . '(`' . $index_fields . "`)\n";
      }
    }
    $sql = 'CREATE TABLE IF NOT EXISTS `' . $table_name . "` (\n";
    $sql .= implode(",\n", $fields) . "\n";

    $sql .= ");\n";
    return $sql;
  }

  public function create_sql_from_trellis($trellis) {
    if (!$trellis) {
      if (!$this->trellis)
        throw new Exception("No valid trellis to generate sql from.");

      $trellis = $this->trellis;
    }

    $core_properties = $trellis->get_core_properties();
    if (count($core_properties) == 0) {
      throw new Exception("Cannot create a table for $trellis->name.  $trellis->name does not have any core properties.");
    }
    $fields = array();
    foreach ($core_properties as $property) {
      $field = $this->properties[$property->name];

      // Don't duplicate shared fields.
      if ($field && $field->share)
        continue;

      $field = array(
          'name' => $property->get_field_name(),
          'type' => $property->get_field_type(),
      );

      if (isset($property->default))
        $field['default'] = $property->default;

      $fields[] = $field;
    }

    if (is_array($this->primary_keys) && count($this->primary_keys) > 0)
      $primary_keys = $this->primary_keys;
    else
      $primary_keys = array($trellis->primary_key);

    return Table::create_sql_from_array($this->name, $fields, $primary_keys, $this->indexes);
  }

  static function format_value($value) {
    if (is_string($value))
      return "'" . $value . "'";

    if ($value === null)
      return 'NULL';

    if ($value === true)
      return 'TRUE';

    if ($value === false)
      return 'FALSE';

    return $value;
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

  static function exists($db, $name) {
    $name = preg_replace('/[^\w]/', '', $name);
    $result = $db->query_values("SHOW TABLES LIKE '$name'");
    return count($result) > 0;
  }

  function load_from_database() {
    $this->properties = Table::load_fields($this->ground->db, $this->name);
    $this->indexes = Table::load_indexes($this->name, $this->ground->db);
  }

  function load_from_schema($source) {
    // Note: not all tables need a reciprical trellis.
    if (isset($this->ground->trellises[$this->name])) {
      // Create bi-connection.  For now I don't know of any case where
      // tables and trellises would be dynamically modified, so I'm not 
      // using MetaHub connections for this.
      $this->trellis = $this->ground->trellises[$this->name];
      $this->trellis->table = $this;
      $this->name = $this->trellis->get_plural();
    }

    MetaHub::extend($this, $source);
    $this->properties = (array) $this->properties;
//    if (isset($source->name)) {
//      $this->name = $source->name;
//    }
  }

  static function load_indexes($name, $db) {
    $rows = $db->query_objects('SHOW INDEX FROM ' . $name);
    $result = array();
    foreach ($rows as $row) {
      $name = $row->Key_name;

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

  function get_data() {
    $result = new stdClass();
    if ($this->name)
      $result->name = $this->name;

    if ($this->primary_keys)
      $result->primary_keys = $this->primary_keys;

    $result->fields = $this->properties;

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

