<?php

class CCK extends Ground_Module {

  static $field_types = array(
      'number_integer' => 'int',
      'nodereference' => 'reference',
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

  static function convert_cck_field_type($type) {
    return CCK::$field_types[$type];
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
        $source = new stdClass();
        $source->type = CCK::convert_cck_field_type($field->type);
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

        $property = new Bloom_Property($field->name, $source, $trellis);
        $trellis->add_property($field->name, $property);
        $table_property = new stdClass();
        $table_property->name = 'field_' . $field->name . '_value';
        $table->properties[$field->name] = $table_property;

        if ($field->type == 'nodereference' || $field->type == 'userreference') {
          $property->override_field('link_class', 'CCK_Link');
        }
      }
    }

    return $result;
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