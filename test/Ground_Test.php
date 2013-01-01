<?php

class Ground_Test extends Ground_Test_Fixtures {
  function setUp() {
    $this->ground = new Ground();
  }

  function test_parent_associations() {
    $this->fixture_load_schemas();
    
  }

}
