<?php

class History extends Ground_Module {
  public function initialize($ground) {
    parent::initialize($ground);
    $this->listen($ground, 'updated', 'on_updated');
  }

  function on_updated($seed, $trellis) {
    
  }

  function auto_commit() {
    $commit = new History_Commit();


    $commit->save();
  }

}
