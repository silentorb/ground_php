<?php

module_load_include('inc', 'ground_drupal', 'inc/CCK_Trellis');

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

  function add_content_types($ground) {
    $types = $this->read_cross_tables();
    $this->convert_cck_joins_to_trellises($types, $ground);
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

    foreach ($content_types as $name => $fields) {
      if (isset($ground->trellises[$name]))
          continue;
      
      $cck_trellis = new CCK_Trellis($ground, $name, $this->field_types, 'content_type_');
      $cck_trellis->trellis->set_parent($node);
      $attributes = new stdClass();
      $attributes->type = 'int';
      $trellis = $cck_trellis->trellis;
      $trellis->add_property('vid', $attributes);
      $cck_trellis->create_properties($fields);
      $result['trellises'][$name] = $cck_trellis->trellis;
      $result['tables'][$name] = $cck_trellis->table;
      $this->trellises[$name] = $cck_trellis->trellis;

      $cck_trellis->table->indexes['vid'] = array(
          'unique' => true,
          'fields' => array(
              'vid',
          ),
      );
    }
    return $result;
  }

  function convert_cck_joins_to_trellises($content_types, $ground) {
    if (!$ground)
      throw new Exception('Invalid Ground Object.');

    $result = array();
    $node = $ground->trellises['node'];
    if (!$node)
      throw new Exception("Node table missing.  It must be loaded before the CCK tables are loaded.");

    foreach ($content_types as $name => $field) {

      $new_field = new stdClass();
      $new_field->name = $field->name;
      $new_field->type = $field->type;
      $cck_trellis = new CCK_Trellis($ground, $field->name, $this->field_types, 'content_field_');
      $attributes = new stdClass();
      $attributes->type = 'int';
      $trellis = $cck_trellis->trellis;
      $trellis->add_property('nid', $attributes);
      $trellis->add_property('vid', $attributes);

      // This is not accurate but practical.  CCK's real primary keys are "vid, delta".
      if ($field->multiple) {
        $trellis->add_property('delta', $attributes);
        $cck_trellis->table->primary_keys = array('vid', 'delta');
      }

      $cck_trellis->table->indexes['nid'] = array(
          'unique' => false,
          'fields' => array(
              'nid',
          ),
      );

      $trellis->primary_key = 'nid';
      $cck_trellis->create_properties(array($new_field), true);

      $result['trellises'][$name] = $trellis;
      $result['tables'][$name] = $cck_trellis->table;
      $this->trellises[$name] = $trellis;
    }
    return $result;
  }

//  function load_trellises($ground) {
//    $types = $this->read_content_types();
//    $model = $this->convert_cck_to_trellises($types, $this->ground);
//    $ground->load_trellises($model['trellises']);
//  }

  function read_content_types() {
    $sql = <<<SQL
SELECT
info.field_name as name,
info.type,
info.multiple,
info.db_storage,
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

  function read_cross_tables() {
    $sql = <<<SQL
SELECT
field.field_name as name,
field.type,
field.multiple,
field.db_storage,
field.required
FROM content_node_field field
WHERE field.db_storage = 0
SQL;
    $db = $this->ground->db;

    $rows = $db->query_objects($sql);
    $fields = array();

    // Group the content types
    foreach ($rows as $field) {
      $field->name = substr($field->name, 6);
      $fields[$field->name] = $field;
    }

    return $fields;
  }

}

/*

$ground = new Ground('default2');
$drupal = $ground->add_module('Ground_Drupal');
print_r($drupal->read_content_types());

 */