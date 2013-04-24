<?php

class Songbird extends Ground_Module {
  function initialize($ground) {
    parent::initialize($ground);
    $ground->add_module('Ground_Drupal');
    $path = drupal_get_path('module', 'songbird');
    $ground->load_schema_from_file($path . '/json/songbird.json');
    $data = array(
        'type' => 'reference',
        'trellis' => 'song'
    );
    $ground->trellises['user']->add_property('songs', $data);    
  }
  
  public function sing() {
    
  }

}
