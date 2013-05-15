<?php

class Drupal_History extends Ground_Module {

function initialize($ground) {
    parent::initialize($ground);
    $ground->add_module('History');
  }
}