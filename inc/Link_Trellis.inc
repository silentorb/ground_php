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

  public function generate_join($id) {
    $sql = "JOIN %table_name ON %table_name.%first_key = $id" .
            " AND %table_name.%second_key = %second_id\n";
    return $this->populate_sql($sql);
  }

  public function generate_delete_row($first_id, $second_id) {
    $sql = "DELETE FROM %table_name WHERE %table_name.%first_key = %second_id" .
            " AND %table_name.%second_key = %first_id\n";
    return $this->populate_sql($sql);
  }

  public function generate_insert($first_id, $second_id) {
    $sql = "INSERT INTO %table_name (%first_key, %second_key) VALUES ($first_id, $second_id)\n;";
    return $this->populate_sql($sql);
  }

  public function get_arguments($property) {
    $this->first_property = $other_table->get_primary_property();
    $this->second_property = $property->parent->get_primary_property();
    $other_table = $this->second_property->parent->get_table_name();
    $temp = array($other_table, $this->first_property->parent->get_table_name());
    sort($temp);
    $this->table_name = implode('_', $temp);
    
    return array(
        '%first_id' => $this->first_property->query(),
        '%second_id' => $id = $this->second_property->query(),
        '%table_name' => $this->table_name,
        '%first_key' => $this->first_property->parent->name . $this->id_suffix,
        '%second_key' => $this->second_property->parent->name . $this->id_suffix,
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

