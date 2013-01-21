<?php

class Ground_Drupal extends Ground_Module {
  function initialize($ground) {
    parent::initialize($ground);
    $path = drupal_get_path('module', 'ground_drupal');
    $this->ground->load_schema_from_file($path . '/ground_drupal.json');
  }

  function read_cck_trellises() {
    $sql = <<<SQL

SQL;
//    $db = $this->ground->db
  }
}
