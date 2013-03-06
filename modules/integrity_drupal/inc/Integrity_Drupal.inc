<?php

class Integrity_Drupal extends Ground_Module {
  function initialize($ground) {
    parent::initialize($ground);
    $this->ground->add_module('Integrity');
  }

}
