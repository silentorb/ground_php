<?php

class Ground_Drupal extends Ground_Module {
  function initialize($ground) {
    parent::initialize($ground);
    $info = new stdClass();
    $info->parent = 'int';
    $this->ground->property_types['file'] = new Property_Type('file', $info, $this->ground->property_types);
    $path = drupal_get_path('module', 'ground_drupal');
    $this->ground->load_schema_from_file($path . '/json/ground_drupal.json');

    $this->listen($ground, 'create.node', 'on_create_node');
  }

  static function add_user($ground, $name, $password, $email, $roles = array()) {
    return $ground->insert_object('user', array(
            'name' => $name,
            'password' => md5($password),
            'email' => $email,
        ));
  }

  static function load_module_json($ground, $module_name, $filename = null) {
    $path = drupal_get_path('module', $module_name);
    if ($filename === null)
      $filename = $module_name . '.json';

    $ground->load_schema_from_file($path . '/' . $filename);
  }

  static function create_node($ground, $arguments) {
    if (!isset($arguments['type']))
      throw new Exception('Node creation requires a type.');

    if (!isset($arguments['uid']))
      throw new Exception('Node creation requires a uid.');

    $node = $ground->insert_object($arguments['type'], $arguments);
    $arguments['nid'] = $node->nid;
    $revision = $ground->insert_object('node_revision', $arguments);
    $node->vid = $revision->vid;
    $ground->db->query('UPDATE node SET vid = ? WHERE nid = ?', array($revision->vid, $node->nid));
    return $node;
  }

  static function copy_nodes($source, $target) {
    Ground_Drupal::configure_files_expansion($source);
    $query = new Content_Query($source->trellises['node']);
    $objects = $query->run();
    $target->update_objects($objects);
  }

  static function configure_files_expansion($ground) {
    $expansion = new stdClass();
    $ground->expansions[] = '/.*images_fid/';
  }

  function on_create_node($node) {
    $revision = $this->ground->insert_object('node_revision', $node);
    $this->ground->db->query('UPDATE node SET vid = ? WHERE nid = ?', array($revision->vid, $node->nid));
  }

}
