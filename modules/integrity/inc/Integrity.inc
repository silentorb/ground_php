<?php

class Integrity extends Ground_Module {
  public function find_trellises_missing_tables() {
    $result = array();
    foreach ($this->ground->trellises as $trellis) {
      if (!Table::exists($this->ground->db, $trellis->get_table_name())) {
        $result[] = $trellis;
      }
    }
    return $result;
  }

  public function create_missing_tables() {
    $trellises = $this->find_trellises_missing_tables();
    foreach ($trellises as $trellis) {
      $this->ground->db->create_table($trellis);
    }
  }

}
