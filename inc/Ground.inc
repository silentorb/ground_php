<?php

if (!function_exists('str_to_bool')) {
  function str_to_bool($string) {
    $string = strtolower($string);
    return $string == 'true' || $string == '1' ? true : false;
  }

}

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

  public function get_field_type() {
    if (isset($this->field_type)) {
      return $this->field_type;
    }

    if ($this->parent) {
      return $this->parent->get_field_type();
    }

    throw new Exception("$this->name could not find valid field type.");
  }

}

class Ground extends Meta_Object {

  public $trellises = array();
  public $map = array();
  public $queries = array();
  public $property_types = array();
  public $db;
  // Eventually Ground MetaHub will support optimize_getter()
  public $modules = array();
  // A single, global instance of Ground used for Vineyard.  I try to keep minimize static members
  // as possible but there are some cases where it is too practical to ignore.
  public static $vineyard_ground;
  // This is not used directly in this class but by ground.module
  public static $default_database = 'ground_test';
  // For general queries, this is an array of object relational paths
  // that point to references that should be expanded to full objects instead of
  // simply an id.  Sort of like explicit lazy loading.
  public $expansions = array();

  public function __construct($database = null) {
    if ($database) {
      $this->db = new Ground_Database();
      $this->db->connect($database);
    }
    $path = drupal_get_path('module', 'ground_php');
    $this->listen($this, 'connect.query', 'on_connect_query');
    $this->load_property_types($path . '/property_types.json');
  }

  public function add_module($name) {
    if (array_key_exists($name, $this->modules))
      return $this->modules[$name];

    $module = new $name();
    $this->connect($module, 'module', 'ground');
    $this->modules[$name] = $module;
    return $module;
  }

  public function add_trellis($name, $object = null, $initialize_parent = true) {
    if (get_class($object) === 'Trellis') {
      $trellis = $object;
      $trellis->ground = $this;
    }
    else {
      $trellis = new Trellis($name, $this);
      if ($object)
        $trellis->load_from_object($object);

      if (!isset($trellis->id)) {
        $trellis->id = count($this->trellises);
      }
    }
    $this->trellises[$name] = $trellis;

    if ($initialize_parent)
      $this->initialize_trellises(array($trellis), $this->trellises);

    return $trellis;
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
      // Ignore objects that already have objects as parents.
      if ($object->parent && !is_object($object->parent)) {
        if (!$all[$object->parent]) {
          throw new Exception("$object->name referencing non-existant parent: $object->parent.");
        }
        $object->set_parent($all[$object->parent]);
        $object->check_primary_key();
      }
    }

    // Now core properties are being set on the fly.
//    foreach ($subset as $object) {
//      $object->update_core_properties();
//    }
  }

  static function load_json_from_file($filename) {
    $json = file_get_contents($filename);
    if ($json === false)
      throw new Exception('Could not find schema file: ' . $filename);

    $data = json_decode($json);
    if (!is_object($data)) {
      throw new Exception("Invalid JSON in file $filename.");
    }

    return $data;
  }

  public function load_schema_from_file($schema_file) {
    $data = Ground::load_json_from_file($schema_file);
    $this->parse_schema($data);
  }

  public function parse_schema($data) {

    if (isset($data->trellises)) {
      $this->load_trellises($data->trellises);
    }

    if (isset($data->views)) {
      $this->views = $data->views;
    }

    if (isset($data->tables)) {
      $this->load_tables($data->tables);
    }
  }

  public function load_property_types($filename) {
    $json = file_get_contents($filename);
    $property_types = json_decode($json);

    foreach ($property_types as $name => $info) {
      $type = new Property_Type($name, $info, $this->property_types);
      $this->property_types[$name] = $type;
    }
  }

  public function load_schema_from_database() {

// The SQL would be a lot simpler except we are converting
// ids to name keys.

    $sql = <<<SQL
SELECT trellis.id, trellis.name, trellis.plural, trellis.primary_key, parent_trellis.name as parent
FROM vineyard_trellises trellis
LEFT JOIN vineyard_trellises parent_trellis ON trellis.parent = parent_trellis.id
SQL;
    $rows = $this->db->query_objects($sql);
    $trellises = array();

    $sql = <<<SQL
SELECT property.name, property.type, property.readonly,
property.insert_trellis, other_trellis.name as trellis
FROM vineyard_properties property
LEFT JOIN vineyard_trellises other_trellis ON property.trellis = other_trellis.id
WHERE property.trellis = ?
SQL;

    foreach ($rows as $row) {
      $properties = $this->db->query_objects($sql, array($row->id));
      $row->properties = array();
      if (!$row->plural)
        unset($row->plural);

      foreach ($properties as $property) {
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

//  public function load_map($map_file) {
//    $json = file_get_contents($map_file);
//    $data = json_decode($json);
//
//    if (isset($data->map)) {
//      foreach ($data->map as $key => $map) {
//        $trellis = $this->trellises[$key];
//        if ($trellis) {
//          foreach ($map->fields as $key => $field_name) {
//            $property = $trellis->all_properties[$key];
//            if ($property) {
//              $property->field_name = $field_name;
//            }
//          }
//        }
//      }
//    }
//  }

  public function load_tables($tables) {
    foreach ($tables as $key => $object) {
      $table = new Table($key, $this);
      $table->load_from_schema($object);
      $this->tables[$key] = $table;
    }
  }

  public function load_trellises($trellises) {
    $subset = array();
    foreach ($trellises as $name => $object) {
      $trellis = $this->add_trellis($name, $object, false);
      $subset[$name] = $trellis;
    }

    $this->initialize_trellises($subset, $this->trellises);
  }

  public function create_query($trellis, $include_links = true, $base_path = '') {
    if (is_string($trellis)) {
      if (!$this->trellises[$trellis])
        throw new Exception("Class '" . $trellis . "' does not exist");
      $trellis = $this->trellises[$trellis];
    }
    else if (!is_object($trellis)) {
      throw new Exception('Invalid trellis passed to create query.');
    }

    foreach ($this->queries as $key => $query) {
      if ($trellis->is_a($key)) {
        return new $query($trellis, $include_links, $base_path);
      }
    }

    return new Query($trellis, $include_links, $base_path);
  }

  function convert_value($value, $type) {
    if ($value == null) {
      if ($type == 'bool')
        return false;

      return null;
    }

    if (array_key_exists($type, $this->property_types)) {
      if ($this->property_types[$type]->parent) {
        return $this->convert_value($value, $this->property_types[$type]->parent->name);
      }
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
      case 'float':
      case 'double':
        return (float) $value;
        break;
    }

    return null;
  }

  // Deprecated in favor of update_object():
  function insert_object($trellis, $data = array()) {
    return $this->update_object($trellis, $data);
  }

  function update_object($trellis, $data = null) {
    $trellis = $this->sanitize_trellis_argument($trellis);
    if (is_array($data))
      $data = (object) $data;

    // If $_deleted is an object than it is a list of links to delete
    // which will be handled by Update.
    // If $data is simply TRUE than the object itself is marked for deletion.
    if ($data->_deleted && !is_object($data->_deleted)) {
      $delete = new Delete();
      $delete->run($trellis, $data);
    }
    else {
      $object = $trellis->create_object();
      MetaHub::extend($object, $data);
      $update = new Update($trellis, $object, $this);
      $result = $update->run();
      return $object;
    }
  }

  function update_objects($objects) {
    foreach ($objects as $seed) {
      $seed = (object) $seed;
      $result = $this->update_object($seed->trellis, $seed);
    }
  }

  public function prepare_for_serialization() {
    $result = new stdClass();
    $result->trellises = array();
    foreach ($this->trellises as $key => $trellis) {
      $result->trellises[$key] = $trellis->get_data();
    }

    if ($this->views)
      $result->views = $this->views;

    return $result;
  }

  public function to_json() {
    $result = $this->prepare_for_serialization();
    return json_encode($result);
  }

  private function sanitize_trellis_argument($trellis) {
    if (!$trellis)
      throw new Exception("insert_object called with undefined trellis.");

    if (is_string($trellis)) {
      $trellis_name = $trellis;
      $trellis = $this->trellises[$trellis_name];
      if (!$trellis)
        throw new Exception("Trellis $trellis_name not found.");
    }

    return $trellis;
  }

}
