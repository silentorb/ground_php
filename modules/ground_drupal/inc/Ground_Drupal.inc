<?php

class Ground_Drupal extends Ground_Module {
  function initialize($ground) {
    parent::initialize($ground);
    $path = drupal_get_path('module', 'ground_drupal');
    $this->ground->load_schema_from_file($path . '/ground_drupal.json');
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

}
