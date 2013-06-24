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
    $this->listen($ground, 'update.node', 'on_update_node');
    $this->listen($ground, 'construct.query.node', 'on_query_node');
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
    unset($node->vid);
    $revision = $this->ground->insert_object('node_revision', $node);
    // Set the node's vid so any following tables that are attached to it
    // such as the cck tables can have access to it.
    $node->vid = $revision->vid;
    $this->ground->db->query('UPDATE node SET vid = ? WHERE nid = ?', array($revision->vid, $node->nid));
  }

  function on_query_node($query, $request) {
    $query->add_join('JOIN node_revisions ON node.nid = node_revisions.nid AND node.vid = node_revisions.vid');
    $query->add_field('node_revisions.body');
    $query->add_field('node_revisions.teaser');
    $query->add_field('node_revisions.log');
    $query->add_field('node_revisions.format');
  }
  
  function on_update_node($node) {
//    unset($node->vid);    
  }

}
