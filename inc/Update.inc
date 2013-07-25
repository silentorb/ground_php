<?php

class Update extends Meta_Object {

  protected $seed;
  protected $fields = array();
  public $overwrite = true;
  public $trellis;
  public $main_table = 'node';
  public $ground;
  public $db;
  static public $log_queries = false;

  public function __construct($trellis, $seed, $ground = null) {
    $this->seed = (object) $seed;
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
      $table = $this->ground->tables[$trellis->name];
      if ($table && is_array($table->primary_keys) && count($table->primary_keys) > 0) {
        $primary_keys = $table->primary_keys;
      }
      else {
        $primary_keys = array($trellis->primary_key);
      }
      $conditions = array();
      $ids = array();
      foreach ($primary_keys as $key) {
        $ids[$key] = $this->seed->{$key};
        $conditions[] = $key . ' = ' . $trellis->properties[$key]->get_field_value($ids[$key]);
      }
      $condition_string = implode(' AND ', $conditions);
      if (!$condition_string) {
        throw new Exception('Condition string cannot be empty.');
      }

      $sql = 'SELECT ' . implode(', ', $primary_keys) . ' FROM ' . $trellis->get_table_name()
          . ' WHERE ' . $condition_string;

      $id_result = $this->db->query_array($sql);
      if (!$id_result) {
        return $this->create_record($trellis);
      }
      return $this->update_record($trellis, $id, $condition_string);
    }
  }

  protected function create_record($trellis) {
    $fields = array();
    $values = array();
    if ($trellis->name == 'node_revision') {
      $trellis = $trellis;
    }
    $core_properties = $trellis->get_core_properties();
    foreach ($core_properties as $property) {

      // Ignore shared fields
      $field = $property->get_field_override();
      if ($field && $field->share)
        continue;

      // Virtual properties aren't saved to the database.
      // Useful when you define custom serialization hooks
      if ($field->virtual)
        continue;

      if (property_exists($this->seed, $property->name)
          || $property->insert_trellis
          || $property->type == 'created'
          || $property->type == 'modified'
          || $property->insert == 'author'
          || $property->insert == 'trellis') {
        $fields[] = '`' . $property->get_field_name() . '`';
        $value = $this->get_field_value($property);
        if (strlen($value) == 0) {
//          $this->get_field_value($property);
          throw new Exception("Field value was empty for inserting $property->name in $trellis->name.");
        }

        $values[] = $value;
      }
    }

    $field_string = implode(', ', $fields);
    $value_string = implode(', ', $values);
    $sql = 'INSERT INTO ' . $trellis->get_table_name() . " ($field_string) VALUES ($value_string);\n";
    if (Update::$log_queries)
      echo $sql . "\n";

    $this->db->query($sql);

    if (isset($this->seed->{$trellis->primary_key})) {
      $id = $this->seed->{$trellis->primary_key};
    }
    else {
      $id = (int) $this->db->last_insert_id($trellis->primary_key);
      $this->seed->{$trellis->primary_key} = $id;
    }

    $this->update_links($trellis, $id, true);
    $this->ground->invoke('create.' . $trellis->name, $this->seed, $trellis);
    return $sql;
  }

  protected function update_record($trellis, $id, $key_condition) {
    $updates = array();
    $core_properties = $trellis->get_core_properties();
    foreach ($core_properties as $property) {
      // Ignore these with updates
      if ($property->name == $trellis->primary_key || $property->type == 'created' || $property->insert == 'alter')
        continue;

      // Ignore shared fields
      $field = $property->get_field_override();
      if ($field && $field->share)
        continue;

      // Virtual properties aren't saved to the database.
      // Useful when you define custom serialization hooks
      if ($field->virtual)
        continue;

      if (property_exists($this->seed, $property->name)
          || $property->insert_trellis
          || $property->insert == 'trellis'
          || $property->type == 'modified') {
        $field = '`' . $property->get_field_name() . '`';
        $value = $this->get_field_value($property);
        $updates[] = "$field = $value";
      }
    }

    // Check if there's nothing to add.
    if (count($updates) !== 0) {


      $update_list_string = implode(', ', $updates);
      $table_name = $trellis->get_table_name();
      $primary_key = $trellis->query_primary_key();

      $sql = <<<SQL
UPDATE $table_name
SET $update_list_string
WHERE $key_condition

SQL;

      if (Update::$log_queries)
        echo $sql . "\n";

      $this->db->query($sql);
    }

    $this->update_links($trellis, $id);
    $this->invoke('updated', $this->seed, $trellis);
    $this->ground->invoke('updated.' . $trellis->name, $this->seed, $trellis);
    return $sql;
  }

  protected function apply_insert($property, $value) {
    if ($property->insert_trellis || $property->insert == 'trellis') {
      return $this->trellis->name;
    }

    if ($property->type == 'created') {
      return time();
    }
    if ($property->type == 'modified') {
      return time();
    }
    if (!$value && $property->insert == 'author') {
      global $user;
      return $user->uid;
    }

    return $value;
  }

  protected function get_field_value($property) {
    $value = $this->seed->{$property->name};

    $value = $this->apply_insert($property, $value);
    $this->seed->{$property->name} = $value;

    $result = $property->get_field_value($value);

    return $result;
  }

  protected function update_links($trellis, $id, $create = false) {
    $links = $trellis->get_links();
    foreach ($links as $property) {
      switch ($property->get_relationship()) {
        case RELATIONSHIP_ONE_TO_ONE:
          // I think the new code I added earlier in the pipeline should
          // completely take care of this so it is no longer needed.
//          $this->update_reference($property, $id);
          break;
        case RELATIONSHIP_ONE_TO_MANY:
          $this->update_one_to_many($property, $id);
          break;
        case RELATIONSHIP_MANY_TO_MANY:
          $this->update_many_to_many($property, $id, $create);
          break;
      }
//      if ($property->type == 'reference') {
//        $this->update_reference($property, $id);
//      }
//      else {
//        $other_property = $property->get_other_property();
//        if ($other_property->type == 'list')
//          $this->update_many_to_many($property, $id, $create);
//        else
//          $this->update_one_to_many($property, $id);
//      }
    }
  }

  protected function update_many_to_many($property, $id, $create = false) {
    $list = $this->seed->{$property->name};
    if (!is_array($list))
      return;

    $link_class = $property->get_link_class();
    $join = new $link_class($property);
    $currently_in_table = array();

    $other_trellis = $property->get_referenced_trellis();

    if (!$create) {
      $rows = $join->query_rows($id, $this->db);

      foreach ($rows as $other_id) {
        if (!in_array($other_id, $list)) {
          $this->db->query($join->generate_delete_row($id, $other_id));
        }
        else {
          $currently_in_table[] = $other_id;
        }
      }
    }

    foreach ($list as $other_id) {
      $object = $other_id;
      $other_id = $other_trellis->get_id($object);
      if ($other_id === null) {
        $object = $this->ground->update_object($other_trellis, $object);
        $other_id = $other_trellis->get_id($object);
      }
      if (!in_array($other_id, $currently_in_table)) {
        $this->db->query($join->generate_insert($other_id, $id));
      }
    }
  }

  protected function update_one_to_many($property, $id) {
    $seed = $this->seed;
    $list = $this->seed->{$property->name};
    if (!is_array($list))
      return;

    foreach ($list as $object) {
      $this->update_reference_object($object, $property, $id);
    }
  }

  protected function update_reference($property, $id) {
    $object = $this->seed->{$property->name};
    if (is_array($object)) {
      $object = (object) $object;
    }
    else if (!is_object($object)) {
      return;
    }

    $this->update_reference_object($object, $property, $id);
  }

  protected function update_reference_object($object, $property, $id) {
    if (is_array($object))
      $object = (object) $object;

    if ($object->trellis) {
      $trellis = $object->trellis;
    }
    else {
      $trellis = $property->trellis;
    }

    $other_property = $property->get_other_property();
    if ($other_property && !isset($object->{$other_property->name})) {
      $object->{$other_property->name} = $id;
    }
    $this->ground->update_object($trellis, $object);
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
