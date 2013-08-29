<?php

//namespace silentorb\ground;

require_once dirname(__FILE__) . '/Query.inc';

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
  public $views = array();
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
  // These might be better called query templates.
  public $static_queries = array();
  // Attach a Fortress object to apply access restriction to query and update services
  public $fortress;
  public $public_vineyard_services = false;

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

  static function remove_private_fields_from_array(&$fields, $trellis) {
    foreach ($fields as $key => $field) {
      if (isset($trellis->properties[$key]) && $trellis->properties[$key]->private) {
        unset($fields[$key]);
      }
    }

    return $fields;
  }

  static function remove_private_and_readonly_fields_from_object($object, $trellis) {
    foreach ($object as $key => $field) {
      if (isset($trellis->properties[$key])) {
        if ($trellis->properties[$key]->private || $trellis->properties[$key]->readonly) {
          unset($object->{$key});
        }
      }
    }

    return $fields;
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

  public function module($name) {
    return $this->add_module($name);
  }

  public function parse_schema($data) {
    if (isset($data->trellises)) {
      $this->load_trellises($data->trellises);
    }

    if (isset($data->views)) {
      $this->views = array_merge($this->views, (array) $data->views);
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

  public function create_query($trellis, $base_path = '') {
    $trellis = $this->sanitize_trellis_argument($trellis, true);

    if ($trellis->is_static) {
      return $trellis->create_query($base_path);
    }
//    foreach ($this->queries as $key => $query) {
//      if ($trellis->is_a($key)) {
//        return new $query($trellis, $include_links, $base_path);
//      }
//    }

    return new \ground\Query($trellis, $base_path);
  }

  public function create_static_query($query_name, $trellis) {
    $trellis = $this->sanitize_trellis_argument($trellis, false);
    $result = new Static_Query($query_name, $trellis);
    $this->static_queries[$query_name] = $result;
    return $result;
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
      case 'list':
      case 'reference':
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

  function delete_object($trellis, $data) {
    $trellis = $this->sanitize_trellis_argument($trellis, false);
    $delete = new Delete();
    $delete->run($trellis, $data);
  }

  function insert_object($trellis, $data = array()) {
    return $this->update_object($trellis, $data);
  }

  function update_object($trellis, $seed, $as_service = false) {
    $trellis = $this->sanitize_trellis_argument($trellis, false);
    if (is_array($seed))
      $seed = (object) $seed;

    // If $_deleted is an object then it is a list of links
    // to delete which will be handled by Update.
    // If $data is simply TRUE then the seed itself is marked for deletion.
    if ($seed->_deleted && !is_object($seed->_deleted)) {
      $this->delete_object($trellis, $seed);
      return new stdClass();
    }
    else {
      // This maybe should eventually be placed in the Update class,
      // but right now I see this more as a preprocessor for services
      // and not as low level as the 'updated' event which is
      // a part of the Update class.
      $this->invoke('update.' . $trellis->name, $seed, $trellis);

      $update = new Update($trellis, $seed, $this);
      $update->is_service = $as_service;
      $result = $update->run();
      return $seed;
    }
  }

  protected function update_object_with_fortress($seed) {
    $seed = (object) $seed;
    if (!isset($seed->trellis) && !isset($this->trellises[$seed->trellis])) {
      return 'Missing Trellis';
    }

    $trellis = $this->trellises[$seed->trellis];
    if (is_object($this->fortress)) {
      $id = $seed->{$trellis->primary_key};
      if (!$id && $id !== 0) { // Create mode
        $access = $this->fortress->creation_access(null, $trellis);
        if (!$access) {
          return 'No creation access';
        }
      }
      else {
        $query = $this->create_query($trellis);
        $query->add_filter($trellis->query_primary_key() . ' = ' . $id);
        $original = $query->run_single();
        $access = $this->fortress->access(null, $original);
        if ($access < 2) {
          return 'No update access: ' . $trellis->query_primary_key() . ' = ' . $id;
        }
      }
    }

    return $this->update_object_with_service($trellis, $seed);
  }

  function update_object_with_service($trellis, $seed) {
    Ground::remove_private_and_readonly_fields_from_object($seed, $trellis);
    return $this->update_object($trellis, $seed, true);
  }

  function update_objects($objects) {
    $result = new stdClass();
    $result->successes = array();
    $result->failures = array();
    foreach ($objects as $seed) {
      $seed = $this->update_object_with_fortress((object) $seed);
      if ($seed && !is_string($seed)) {
        $result->successes[] = $seed;
      }
      else {
        $result->failures[] = $seed;
      }
    }

    return $result;
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

  public function to_json($include_tables = false) {
    $result = $this->prepare_for_serialization();

    if ($include_tables) {
      foreach ($this->tables as $key => $table) {
        if ($table->properties)
          $result->tables[$key] = $table->get_data();
      }
    }

    return json_encode($result);
  }

  private function sanitize_trellis_argument($trellis, $include_static_queries = false) {
    if (!$trellis)
      throw new Exception("insert_object called with undefined trellis.");

    if (is_string($trellis)) {
      $trellis_name = $trellis;
      if ($this->static_queries[$trellis_name]) {
        $trellis = $this->static_queries[$trellis_name];
      }
      else {
        $trellis = $this->trellises[$trellis_name];
        if (!$trellis)
          throw new Exception("Trellis $trellis_name not found.");
      }
    }

    return $trellis;
  }

  public function vineyard_service($request) {
    if ($this->irrigation) {
      $channel = $this->irrigation->find_channel($request->path, $request);

      if ($channel) {
        if (is_object($this->fortress)) {
          $access = $this->fortress->channel_access(null, $channel, $request);

          // If not true, $access is a Wall
          if ($access !== true) {
            throw new Exception($access->repel(), 400);
          }
        }
        $result = $channel->run($request);
        if (is_array($result)) {
          $objects = $result;
          $result = new stdClass();
          $result->objects = $objects;
        }

        return $result;
      }
    }

    if (!$this->public_vineyard_services)
      throw new Exception('Path not found.', 404);

    $trellis = $request->path[0];

    if ($trellis === null)
      throw new Exception('No path or trellis was specified.', 404);

    $query = $this->create_query($trellis);
    $this->customize_query_from_request($query, $request);

    return $query->run_as_service(true);
  }

  public function customize_query_from_request($query, $request) {
    $id = $request->path[1];
    $args = $request->arguments;
    $trellis = $request->path[0];

    if ($id) {
      $query->add_filter($query->trellis->query_primary_key() . ' = ' . (int) $id);
    }

    // Currently only works for int types.
    $properties = $query->trellis->get_all_properties();
    foreach ($args as $key => $value) {
      if (array_key_exists($key, $properties)) {
        $property = $properties[$key];
        $query->add_property_filter($property, $value);
      }
      else if ($key == 'order') {
        $order_string = Ground::format_order_string($value, $query->trellis);
        $query->add_post('ORDER BY ' . $order_string);
      }
    }
    $this->invoke('query.' . $trellis, $query, $request);

    if (isset($args['limit'])) {
      $offset = 0;
      if (isset($args['offset']))
        $offset = (int) $args['offset'];

      $query->limit = $query->generate_pager($offset, (int) $args['limit']);
    }

    if (isset($request->POST['query'])) {
      $this->customize_query_from_object($query, $request->POST['query']);
    }
  }

  public function customize_query_from_object($query, $settings) {
    if (isset($settings->expansions)) {
      foreach ($settings->expansions as $expansion) {
        $query->add_expansion($expansion);
      }
    }
    if (isset($settings->filters)) {
      foreach ($settings->filters as $filter) {
        if (!is_object($filter))
          throw new Exception('Invalid filter', 400);

        $properties = $query->trellis->get_all_properties();
        if (!array_key_exists($filter->property, $properties))
          throw new Exception("Invalid trellis property in filter: $filter->property");

        $property = $properties[$filter->property];
        $query->add_property_filter($property, $filter->value, $filter->like);
      }
    }
  }

  public static function format_order_string($string, $trellis) {
    $properties = $trellis->get_all_properties();
    $tokens = explode(' ', $string);
    foreach ($tokens as $key => $token) {
      $text = strtolower($token);
      if ($text != 'desc' && $text != 'asc' && !isset($properties[$token])) {
        $tokens[$key] = '';
      }
    }

    return implode($tokens);
  }

  public static function strip(&$object, $strip_properties = array()) {
    if (count($strip_properties) == 0)
      $strip_properties = array('id');

    if (is_object($object)) {
      $properties = get_object_vars($object);
      foreach ($strip_properties as $name) {
        unset($object->{$name});
      }
      foreach ($properties as $name => $value) {
        if (is_object($value) || is_array($value)) {
          $object->{$name} = Ground::strip($value, $strip_properties);
        }
      }
    }
    else if (is_array($object)) {
      foreach ($strip_properties as $name) {
        unset($object[$name]);
      }
      foreach ($object as $name => $value) {
        if (is_object($value) || is_array($value)) {
          // Limited prevention of loops:
          if ($value === $object)
            continue;

          $object[$name] = Ground::strip($value, $strip_properties);
        }
      }
    }

    return $object;
  }

}
