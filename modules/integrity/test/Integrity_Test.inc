<?php

class Integrity_Test extends Ground_Test_Case {
  function setUp() {
    parent::setUp();
    $this->integrity = $this->ground->add_module('Integrity');
  }

  function test_find_trellises_missing_tables() {
    $this->fixture->load_schemas();
    $trellis = $this->ground->trellises['warrior'];
    
    // Add just one of the trellis tables to the database
    $this->ground->db->create_table($trellis);

    // The remaining tables should show up as missing.
    $missing_tables = $this->integrity->find_trellises_missing_tables();
    $this->assertEquals(count($missing_tables), count($this->ground->trellises) - 1);

    // Fill in the gaps in the database.
    $this->integrity->create_missing_tables();

    // Now all tables should be present.
    $missing_tables = $this->integrity->find_trellises_missing_tables();
    $this->assertEquals(count($missing_tables), 0);
  }

}
