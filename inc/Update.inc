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
    if (is_string($trellis)) {
      if ($ground)
        $this->trellis = $ground->trellises[$trellis];
      else
        throw new Exception('No Ground provided to find trellis string.');
    }
    else {
      $this->trellis = $trellis;
    }

    $this->main_table = $this->trellis->get_table_name();
    if ($ground) {
      $this->ground = $ground;
    }
    else {
      $this->ground = $this->trellis->ground;
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
      $sql = "SELECT $primary_key FROM " . $trellis->get_table_name() . " WHERE $primary_key = $id";
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
    $sql = 'INSERT INTO ' . $trellis->get_table_name() . " ($field_string) VALUES ($value_string);\n";
    $this->invoke('created', $this->seed, $trellis);
    echo $sql;
    $this->db->query($sql);

    // Hard coding conversion to int is a quick fix hack.  Eventually
    // I should convert it based on the type of trellis property.
    $this->seed->{$trellis->primary_key} = $id = (int) $this->db->last_insert_id($trellis->primary_key);
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
    $table_name = $trellis->get_table_name();
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
    $property_name = $this->trellis->name . '->' . $property->name;
    if ($property->insert_trellis) {
      $value = $this->trellis->name;
    }
    else {
      $value = $this->seed->{$property->name};
    }

    if (is_string($value))
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
    if ($property->type == 'reference') {
      // !!! Eventually I'll need to check the to see if the other property is
      // using a primary key other than 'id', but this will work for the moment.
      if (is_object($value)) {
        if (isset($value->id)) {
          $value = $value->id;
        }
        else {
          throw new Exception("Cannot update reference because value for '$property_name' is an object that does not have an id.");
        }
      }
    }
    else if ($value === null) {
      $value = 'NULL';
    }

    if (is_object($value))
      throw new Exception("Property $property_name cannot be an object.");

    if (is_array($value))
      throw new Exception("Property $property_name cannot be an array.");

    return $value;
  }

  protected function update_links($trellis, $id, $create = false) {
    foreach ($trellis->links as $link) {
      if ($link->type == 'reference')
        continue;
      continue;
      $list = $this->seed->{$link->name};
      if (!$list)
        continue;

      $join = new Link_Trellis($link->other->get_primary_property(), $trellis->get_primary_property());
      $currently_in_table = array();

      if (!$create) {
        $rows = $join->query_rows($id);

        foreach ($rows as $other_id) {
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

  public function run($return_sql = false) {
    $result = new stdClass();

    // JOINED tables will require multiple generate_sqls...
    $trellis = $this->trellis;
    $tree = $trellis->get_tree();

    // The sql is only collected for debugging purpose.
    // The individual parts of the generated sql script
    // are executed individually.
    // Eventually this should probably be converted to
    // a transaction.
    $sql = '';
    foreach ($tree as $trellis) {
      $sql .= $this->generate_sql($trellis);
    }

    if ($return_sql)
      $result->sql = $sql;

    $result->seed = $this->seed;
    return $result;
  }

}
