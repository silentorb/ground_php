<?php

class CCK extends Ground_Module {

  public $field_types = array(
      'number_integer' => 'int',
      'nodereference' => 'reference',
      'filefield' => 'file',
      'text' => 'string',
  );
  static $drupal_types = array(
      'node' => 'nid',
      'user' => 'uid',
  );
  public $trellises = array();

  function initialize($ground) {
    parent::initialize($ground);
    $this->ground->add_module('Ground_Drupal');
    $path = drupal_get_path('module', 'ground_drupal');
    $json = file_get_contents($path . '/json/field_types.json');
    $this->field_types = (array) json_decode($json);
  }

  function create_link($trellis, $name, $ground, $field) {
    $property = $trellis->add_property($name . '_list', array(
        'type' => 'list',
        'trellis' => $name, // No easy way to make this plural.  It's a Drupal problem.
        'property' => $field->name
            ));

    $property->override_field('link_class', 'CCK_Link');
    $property->override_field('name', 'field_' . $field->name . '_value');

    if ($field->multiple) {
      $this->create_link_table($trellis, $name, $ground, $field);
    }
  }

  function create_link_table($trellis, $name, $ground, $field) {
    // Use the Ground pointing to the Drupal database, not the passed Ground, which could be different
    $table = new Table('content_field_' . $field->name, $this->ground);
    $table->load_from_database();
    $sql = Table::create_sql_from_array($table->name, $table->properties, array());
    $ground->db->query($sql);
  }

  function add_content_types($ground) {
    $types = $this->read_content_types();
    $this->convert_cck_to_trellises($types, $ground);
  }

  function convert_cck_to_trellises($content_types, $ground) {
    if (!$ground)
      throw new Exception('Invalid Ground Object.');

    $result = array();
    $node = $ground->trellises['node'];
    if (!$node)
      throw new Exception("Node table missing.  It must be loaded before the CCK tables are loaded.");

    foreach ($content_types as $name => $type) {

      // Create Trellis
      $trellis = new Trellis($name, $ground);
      $trellis->set_parent($node);
      $ground->trellises[$name] = $trellis;
      $result['trellises'][$name] = $trellis;
      $this->trellises[$name] = $trellis;

      // Create Table
      $table = new Table('content_type_' . $name, $ground);
      $table->connect_trellis($trellis);
      $ground->tables[$name] = $table;
      $result['table'][$name] = $table;

      // Create Properties
      foreach ($type as $field) {
        $info = $this->field_types[$field->type];
        if (!$info)
          throw new Exception("No CCK field info for $field->type.");

        // One CCK field can map to multiple table fields.  Whatever.
        foreach ($info as $sub_field) {
          $source = new stdClass();
          $source->type = $sub_field->type;
          if ($field->type == 'nodereference') {
            $source->trellis = 'node';
            $source->property = $name . '_list';
            $this->create_link($node, $name, $ground, $field);
          }
          else if ($field->type == 'userreference') {
            $source->trellis = 'user';
            $source->property = $name . '_list';
            $this->create_link($ground->trellises['user'], $name, $ground, $field);
          }

          if ($field->multiple || $field->type == 'userreference')
            $source->type = 'list';

          $sub_field_name = $field->name;
          if (count($info) > 1)
            $sub_field_name = $field->name . '_' . $sub_field->name;

          $property = new Bloom_Property($sub_field_name, $source, $trellis);
          $trellis->add_property($sub_field_name, $property);
          $table_property = new stdClass();
          $table_property->name = 'field_' . $field->name . '_' . $sub_field->name;
          $table->properties[$sub_field_name] = $table_property;

          if ($field->type == 'nodereference' || $field->type == 'userreference') {
            $property->override_field('link_class', 'CCK_Link');
          }
        }
      }
    }
    return $result;
  }

  static function get_field_suffix($type) {
    $types = array(
        'userreference' => 'uid',
        'nodereference' => 'nid',
        'filefield' => 'fid',
    );
    if (array_key_exists($type, $types)) {
      return $types[$type];
    }

    return 'value';
  }

  function load_trellises($ground) {
    $types = $this->read_content_types();
    $model = $this->convert_cck_to_trellises($types, $this->ground);
    $ground->load_trellises($model['trellises']);
  }

  function read_content_types() {
    $sql = <<<SQL
SELECT
info.field_name as name,
info.type,
info.multiple,
info.required,
instance.type_name as trellis
FROM content_node_field_instance instance
JOIN content_node_field info ON info.field_name = instance.field_name
ORDER BY instance.type_name, instance.weight
SQL;
    $db = $this->ground->db;

    $properties = $db->query_objects($sql);
    $content_types = array();

    // Group the content types
    foreach ($properties as $property) {
      $content_types[$property->trellis][$property->name] = $property;
      $property->name = substr($property->name, 6);
      unset($property->trellis);
    }

    return $content_types;
  }

}

/*

$ground = new Ground('default2');
$drupal = $ground->add_module('Ground_Drupal');
print_r($drupal->read_content_types());

 */