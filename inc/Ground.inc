<?php

class Property_Type {

  public $name;
  public $property_class;
  public $field_type;
  public $default = null;

  public function __construct($name, $info, $types) {
    // Transferring parent properties is done before any other assignment
    // so the MetaHub::extend() can be overridden.
    if (isset($info->parent)) {
      $parent = $types[$info->parent];
      MetaHub::extend($this, $parent);
      $this->parent = $parent;
    }
    else {
      $this->field_type = $info->field_type;
    }
    
    $this->name = $name;
    $this->property_class = 'Bloom_Property';
    if (isset($info->default)) {
      $this->default = $info->default;
    }
  }

}

class Ground extends Meta_Object {

  public $trellises = array();
  public $map = array();
  public $queries = array();
  public $property_types = array();
  public $db;

  public function __construct($database = null) {
    if ($database) {
      $this->db = new Ground_Database();
      $this->db->connect($database);
    }
    $path = drupal_get_path('module', 'ground_php');
    $this->load_schema_from_file($path . '/vineyard.json');

    $this->listen($this, 'connect.query', 'on_connect_query');

    $json = file_get_contents($path . '/property_types.json');
    $property_types = json_decode($json);

    foreach ($property_types as $name => $info) {
      $type = new Property_Type($name, $info, $this->property_types);
      $this->property_types[$name] = $type;
    }
  }

  public function add_module($name) {
    $module = new $name();
    $this->connect($module, 'module', 'ground');
  }

  public function on_connect_query($query) {
    $this->listen($query, 'all', 'respond');
  }

  public function respond() {
    $args = func_get_args();
    call_user_func_array(array($this, 'invoke'), $args);
  }

  public function initialize_trellises($subset, $all = null) {
    if ($all === null)
      $all = $subset;

    foreach ($subset as $object) {
// Convert a string reference into a direct reference to the actual object
      if ($object->parent) {
        $object->parent = $all[$object->parent];
        $object->check_primary_key();
      }
    }

    foreach ($subset as $object) {
      $object->update_core_properties();
    }
  }

  public function load_schema_from_file($schema_file) {
    $json = file_get_contents($schema_file);
    if ($json === false)
      throw new Exception('Could not find schema file: ' . $schema_file);
    $this->load_schema($json);
  }

  public function load_schema($json) {
    $data = json_decode($json);

    if (!isset($data->trellises))
      throw new Exception('classes object is required.');

    foreach ($data->trellises as $key => $object) {
      $trellis = new Trellis($key, $this);
      $trellis->load_from_object($object);
      $this->trellises[$key] = $trellis;
    }

    $this->initialize_trellises($this->trellises, $this->trellises);

//    foreach ($this->trellises as $object) {
//      // Convert a string reference into a direct reference to the actual object
//      if ($object->parent) {
//        $object->parent = $this->trellises[$object->parent];
//        $object->check_primary_key();
//      }
//    }
//
//    foreach ($this->trellises as $object) {
//      $object->update_core_properties();
//    }

    if (isset($data->maps))
      $this->maps = $data->maps;
  }

  public function load_schema_from_database() {

// The SQL would be a lot simpler except we are converting
// ids to name keys.

    $sql = <<<SQL
SELECT trellis.id, trellis.name, trellis.plural, trellis.primary_key, parent_trellis.name as parent
FROM vineyard_trellises trellis
LEFT JOIN vineyard_trellises parent_trellis ON trellis.parent = parent_trellis.id
SQL;
    $rows = db_query($sql);
    $trellises = array();

    $sql = <<<SQL
SELECT property.name, property.type, property.readonly,
property.insert_trellis, other_trellis.name as trellis
FROM vineyard_properties property
LEFT JOIN vineyard_trellises other_trellis ON property.trellis = other_trellis.id
WHERE property.trellis = %d
SQL;

    while ($row = db_fetch_object($rows)) {
      $properties = db_query($sql, $row->id);
      $row->properties = array();
      if (!$row->plural)
        unset($row->plural);

      while ($property = db_fetch_object($properties)) {
        $row->properties[$property->name] = $property;
        if ($property->readonly)
          $property->readonly = true;
        if ($property->insert_trellis)
          $property->insert_trellis = true;
      }

      $trellis = new Trellis($row->name, $this);
      $trellis->load_from_object($row);
      $trellises[$row->name] = $trellis;
    }

    $this->trellises = array_merge($this->trellises, $trellises);

    $this->initialize_trellises($trellises, $this->trellises);
  }

  public function load_map($map_file) {
    $json = file_get_contents($map_file);
    $data = json_decode($json);

    if (isset($data->map)) {
      foreach ($data->map as $key => $map) {
        $trellis = $this->trellises[$key];
        if ($trellis) {
          foreach ($map->fields as $key => $field_name) {
            $property = $trellis->all_properties[$key];
            if ($property) {
              $property->field_name = $field_name;
            }
          }
        }
      }
    }
  }

  public function create_query($trellis, $include_links = true) {
    if (is_string($trellis)) {
      if (!$this->trellises[$trellis])
        throw new Exception("Class '" . $trellis . "' does not exist");
      $trellis = $this->trellises[$trellis];
    }
    foreach ($this->queries as $key => $query) {
      if ($trellis->is_a($key)) {
        return new $query($trellis, $include_links);
      }
    }

    return new Query($trellis);
  }

  static function convert_value($value, $type) {
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
      case 'text':
      case 'reference':
        return $value;
        break;
      case 'bool':
        return str_to_bool($value);
        break;
      case 'double':
        return (float) $value;
        break;
    }

    return null;
  }

  public function prepare_for_serialization() {
    $result = new stdClass();
    $result->trellises = array();
    foreach ($this->trellises as $key => $trellis) {
      $result->trellises[$key] = $trellis->get_data();
    }

    return $result;
  }

  public function to_json() {
    $result = $this->prepare_for_serialization();
    return json_encode($result);
  }

}
