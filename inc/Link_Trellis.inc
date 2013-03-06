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
  
  public function create_trellis() {
    $trellis = new Trellis($this->table_name);
    $attributes = new stdClass();
    $attributes->type = 'int';
    $trellis->add_property($this->args['%first_key'], $attributes);
    $trellis->add_property($this->args['%second_key'], $attributes);
    return $trellis;
  }

  public function generate_join($id) {
    $sql = "JOIN %table_name ON %table_name.%first_key = $id" .
            " AND %table_name.%second_key = %back_id\n";
    return $this->populate_sql($sql);
  }

  public function generate_delete_row($first_id, $second_id) {
    $sql = "DELETE FROM %table_name WHERE %table_name.%first_key = $second_id" .
            " AND %table_name.%second_key = %first_id\n";
    return $this->populate_sql($sql);
  }

  public function generate_insert($first_id, $second_id) {
    $sql = "INSERT INTO %table_name (%first_key, %second_key) VALUES ($first_id, $second_id)\n;";
    return $this->populate_sql($sql);
  }

  public function get_arguments($property) {
    $other_property = $property->get_other_property();
    $other_table = $other_property->parent->get_table_name();
    $temp = array($other_table, $property->parent->get_table_name());
    sort($temp);
    $this->table_name = implode('_', $temp);

    return array(
        '%first_id' => $property->query(),
        '%second_id' => $other_property->query(),
        '%table_name' => $this->table_name,
        '%first_key' => $property->parent->name . $this->id_suffix,
        '%second_key' => $other_property->parent->name . $this->id_suffix,
        '%back_id' => $other_property->parent->get_table_name() . '.' . $other_property->parent->primary_key,
    );
  }

  function populate_sql($sql) {
    return str_replace(array_keys($this->args), $this->args, $sql);
  }

  public function query_rows($id) {
    $sql = $this->populate_sql("SELECT %table_name.%first_key FROM %table_name" .
            "WHERE %table_name.%second_key = %second_id\n");
    return $this->db->query_values($sql);
  }

}

