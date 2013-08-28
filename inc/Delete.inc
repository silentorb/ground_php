<?php

class Delete {

  public $max_depth = 20;

  function delete_child($link, $id, $depth = 0) {
    $other_property = $link->get_other_property();
    $other_trellis = $other_property->parent;
    $query = $other_trellis->ground->create_query($other_trellis);
    $query->add_filter($other_property->query() . ' = ' . $id);
    $objects = $query->run();
    foreach ($objects as $object) {
      $this->run($other_trellis, $object, $depth + 1);
    }
  }

  function delete_children($trellis, $id, $depth = 0) {
    $links = $this->get_child_links($trellis);
    foreach ($links as $link) {
      $this->delete_child($link, $id, $depth);
    }
  }

  function delete_record($trellis, $id) {
    $sql = 'DELETE FROM ' . $trellis->get_table_name()
            . "\nWHERE " . $trellis->query_primary_key() . ' = ' . $id;

    $trellis->ground->db->query($sql);
  }

  function get_child_links($trellis) {
    $links = $trellis->get_links();
    foreach ($links as $link) {
      $other = $link->get_other_property();
      if ($other->name == 'parent' || $other->is_parent) {
        $result[] = $link;
      }
    }

    return $result;
  }

  public function run($trellis, $object, $depth = 0) {
    if ($depth > $this->max_depth)
      throw new Exception("Max depth of $this->max_depth exceeded.  Possible infinite loop.");

    $id = $object->{$trellis->primary_key};
    if ($id === null)
      throw new Exception('Object was tagged to be deleted but has no id.');

    $id = (int) $id;
    $tree = $trellis->get_tree();

    foreach ($tree as $trellis) {
      $this->delete_record($trellis, $id);
    }

    $this->delete_children($trellis, $id, $depth);
  }

}
