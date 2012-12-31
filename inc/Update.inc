<?php

class Update extends Meta_Object {

  protected $seed;
  protected $fields = array();
  public $overwrite = true;
  public $trellis;
  public $main_table = 'node';
  public $ground;
  public $db;

  public function __construct($trellis, $seed, $ground = null) {
    $this->seed = $seed;
    $this->trellis = $trellis;
    $this->main_table = $trellis->get_plural();
    if ($ground) {
      $this->ground = $ground;
    }
    else {
      $this->ground = $trellis->ground;
    }

    $this->db = $ground->db;
    $this->connect($this->ground, 'ground', 'query');
  }

  public function generate_sql($trellis) {
    $duplicate = '';

    $id = $this->seed->{$trellis->primary_key};
    if (!$id && $id !== 0) {
      return $this->create_record($trellis);
    }
    else {
      $primary_key = $trellis->query_primary_key();
      $sql = "SELECT $primary_key FROM " . $trellis->get_plural() . " WHERE $primary_key = $id";
      $id = $this->db->query_value($sql);
      if (!$id && $id !== 0) {
        return $this->create_record($trellis);
      }
      return $this->update_record($trellis, $id);
    }
  }

  protected function create_record($trellis) {
    $fields = array();
    $values = array();
    foreach ($trellis->core_properties as $property) {
      if (property_exists($this->seed, $property->name) || $property->insert_trellis) {
        $fields[] = '`' . $property->name . '`';
        $values[] = $this->get_field_value($property);
      }
    }

    $field_string = implode(', ', $fields);
    $value_string = implode(', ', $values);
    $sql = 'INSERT INTO ' . $trellis->get_plural() . " ($field_string) VALUES ($value_string);\n";
    $this->invoke('created', $this->seed, $trellis);
    $this->db->query($sql);

    $this->seed->{$trellis->primary_key} = $id = $this->db->last_insert_id($trellis->primary_key);
    $this->update_links($trellis, $id, true);
    return $sql;
  }

  protected function update_record($trellis, $id) {
    $updates = array();
    foreach ($trellis->core_properties as $property) {
      // Ignore these with updates
      if ($property->name == $trellis->primary_key || $property->type == 'created')
        continue;

      if (property_exists($this->seed, $property->name) || $property->insert_trellis) {
        $field = '`' . $property->name . '`';
        $value = $this->get_field_value($property);
        $updates[] = "$field = $value";
      }
    }

    // Check if there's nothing to add.
    if (count($updates) === 0)
      return '';

    $update_list_string = implode(', ', $updates);
    $table_name = $trellis->get_plural();
    $primary_key = $trellis->query_primary_key();

    $sql = <<<SQL
UPDATE $table_name
SET $update_list_string
WHERE $primary_key = $id

SQL;

    $this->db->query($sql);
    $this->update_links($trellis, $id);
    $this->invoke('updated', $this->seed, $trellis);
    return $sql;
  }

  protected function get_field_value($property) {
    if ($property->insert_trellis) {
      $value = $this->trellis->name;
    }
    else {
      $value = $this->seed->{$property->name};
    }

    $value = str_replace("'", "\\'", $value);

    if ($property->type == 'string' || $property->type == 'text') {
      $value = "'" . preg_replace("/[\r\n]+/", '\n', $value) . "'";
    }
    if ($property->type == 'created') {
      $value = time();
    }
    if ($property->type == 'modified') {
      $value = time();
    }
    else if ($value == null) {
      $value = 'NULL';
    }

    return $value;
  }

  protected function update_links($trellis, $id, $create = false) {
    foreach ($trellis->links as $link) {
      $list = $this->seed->{$link->name};
      if (!$list)
        continue;

      $join = new Link_Trellis($link->other->get_primary_property(), $trellis->get_primary_property());
      $currently_in_table = array();

      if (!$create) {
        $rows = $join->query_rows($id);

        while ($other_id = $this->db->query_value($rows)) {
          if (!in_array($other_id, $list)) {
            $this->db->query($join->delete_row($id, $other_id));
          }
          else {
            $currently_in_table[] = $other_id;
          }
        }
      }

      foreach ($list as $other_id) {
        if (!in_array($other_id, $currently_in_table)) {
          $this->db->query($join->generate_insert($id, $other_id));
        }
      }
    }
  }

  public function run($return_sql) {
    $result = new stdClass();

    // JOINED tables will require multiple generate_sqls...
    $tree = array();
    $trellis = $this->trellis;

    do {
      array_unshift($tree, $trellis);
    }
    while ($trellis = $trellis->parent);

    $sql = '';
    foreach ($tree as $trellis) {
      $sql .= $this->generate_sql($trellis);
    }
//    print $sql;
//    db_query($sql);
    if ($return_sql)
      $result->sql = $sql;

    $result->seed = $this->seed;
//    $result->fields = $this->fields;
    return $result;
  }

}
