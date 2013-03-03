<?php

class CCK_Link extends Link_Trellis {
  static function get_table_name($property_name) {
    
  }

  public function get_arguments($property) {

    $result = array();
    $other_property = $property->get_other_property();
    $other_type = $other_property->parent->name;
    if ($other_type == 'user' || $other_type == 'node') {
      return array(
          '%table_name' => 'content_field_' . $property->name,
          '%first_key' => 'field_' . $property->name . '_' . CCK::$drupal_types[$other_type],
          '%first_id' => $property->parent->query_primary_key(),
          '%second_key' => 'nid',
          '%second_id' => $other_property->parent->query_primary_key(),
          '%back_id' => $other_property->parent->get_table_name() . '.' . $other_property->parent->primary_key,
      );
    }
    else {
      return array(
          '%table_name' => 'content_field_' . $property->name,
          '%first_key' => 'nid',
          '%first_id' => $property->parent->query_primary_key(),
          '%second_key' => 'field_' . $other_property->name . '_' . CCK::$drupal_types[$other_type],
          '%second_id' => $other_property->parent->query_primary_key(),
          '%back_id' => $other_property->parent->get_table_name() . '.' . $other_property->parent->primary_key,
      );
    }
  }

}

