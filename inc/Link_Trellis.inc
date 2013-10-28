<?php

class Link_Trellis {

  public $table_name;
  public $first_property;
  public $second_property;
  public $id_suffix = '';

  public function __construct($property) {
    $other_table = $property->get_referenced_trellis();
    $this->property = $property;
    $this->args = $this->get_arguments($property);
  }

  public function create_trellis($ground) {

    $trellis = new Trellis($this->args['%table_name'], $ground);
    $trellis->plural = $trellis->name;
    $attributes = new stdClass();
    $attributes->type = 'int';
    $trellis->add_property($this->args['%first_key'], $attributes);
    $trellis->add_property($this->args['%second_key'], $attributes);
    return $trellis;
  }

  public function generate_join($id, $reverse = false) {
    if ($reverse) {
      $sql = "JOIN %table_name ON %table_name.%second_key = %back_id" .
              " AND %table_name.%first_key = $id\n";
    }
    else {
      $sql = "JOIN %table_name ON %table_name.%second_key = $id" .
              " AND %table_name.%first_key = %forward_id\n";
    }
    return $this->populate_sql($sql);
  }

  public function generate_delete_row($first_id, $second_id) {
    $sql = "DELETE FROM %table_name WHERE %table_name.%first_key = " . $second_id .
            " AND %table_name.%second_key = $first_id\n";
    return $this->populate_sql($sql);
  }

  public function generate_insert($first_id, $second_id) {
    $sql = "REPLACE INTO %table_name (`%first_key`, `%second_key`) VALUES ($first_id, $second_id)\n;";
    return $this->populate_sql($sql);
  }

  public function get_arguments($property) {
    $other_property = $property->get_other_property();

    // Since we are checking for an ideal cross table name,
    // Use plural trellis names instead of any table name overrides.
    $other_table = $other_property->parent->get_plural();
    $temp = array($other_table, $property->parent->get_plural());
    sort($temp);
    $this->table_name = implode('_', $temp);
    $result = array(
        '%first_id' => $property->query(),
        '%second_id' => $other_property->query(),
        '%back_id' => $other_property->parent->query_primary_key(),
        '%forward_id' => $property->parent->query_primary_key(),
    );

    $ground = $property->parent->ground;
    $table = $ground->tables[$this->table_name];
    if ($table && count($table->properties) >= 2) {
      foreach ($table->properties as $name => $field) {
        if ($field->trellis === $property->trellis) {
          $first_key = $name;
        }
        else if ($field->trellis === $other_property->trellis) {
          $second_key = $name;
        }
      }

      if (!$first_key || !$second_key) {
        throw new Exception("Properties do not line up for cross table $this->table_name.");
      }

      $result = array_merge($result, array(
          '%table_name' => $table->name,
          '%first_key' => $first_key,
          '%second_key' => $second_key,
      ));
    }
    else {
      $result = array_merge($result, array(
          '%table_name' => $this->table_name,
          '%first_key' => $property->parent->name . $this->id_suffix,
          '%second_key' => $other_property->parent->name . $this->id_suffix,
      ));
    }

    return $result;
  }

  function populate_sql($sql) {
    return str_replace(array_keys($this->args), $this->args, $sql);
  }

  public function query_rows($id, $db) {
    $sql = $this->populate_sql("SELECT %table_name.%first_key FROM %table_name " .
            "WHERE %table_name.%second_key = $id\n");
    return $db->query_values($sql);
  }

}

