<?php

class Update_Test extends PHPUnit_Framework_TestCase {
  function setUp() {
    $this->ground = new Ground('ground_test');
    $this->trellis = $this->ground->trellises['vineyard_trellis'];
    $this->object = $this->trellis->create_object();
  }

  function test_insert() {
    $update = new Update($this->trellis, $this->object, $this->ground);
    $result = $update->run(true);
    $this->assertSame($result->seed,$this->object);
    $this->assertSame(1, $result->seed->id);
  }

}
