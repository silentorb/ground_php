<?php

interface Bird {
  public function sing($song);
}

class Songbird extends Ground_Module {

  public $birds = array();

  function initialize($ground) {
    parent::initialize($ground);
    $ground->add_module('Ground_Drupal');
    $path = drupal_get_path('module', 'songbird');
    $ground->load_schema_from_file($path . '/json/songbird.json');

    // Add a backreference to the user table.
    // This will use the songs_users cross table.
    $data = array(
        'type' => 'list',
        'trellis' => 'song'
    );
    $ground->trellises['user']->add_property('songs', $data);
    $this->listen($ground, 'query.song', 'on_query');

    if (function_exists('mimemail')) {
      $email = new Songbird_Email();
      $this->birds['Songbird_Email'] = $email;
    }
  }

  public function record_song($song) {
    if (((int) $song->type) === 0) {
      $song->type = $this->ground->db->query_value('SELECT id FROM song_types WHERE name = ?', array($type));
    }
    $this->ground->insert_object('song', $song);
  }

  public function sing($song) {
    $this->record_song($song);
    $this->sing_chorus($song);
  }

  public function sing_chorus($song) {
    foreach ($this->birds as $bird) {
      $bird->sing($song);
    }
  }

  public function on_query($query, $request) {
    $arguments = $request->arguments;
    $clauses = array();

    if (isset($arguments['start'])) {
      $clauses[] = '`timestamp` >= ' . (int) $arguments['start'];
    }
    if (isset($arguments['end'])) {
      $clauses[] = '`timestamp` <= ' . (int) $arguments['end'];
    }

    if (count($clauses) > 0) {
      $where = implode(' AND ', $clauses);
      $query->add_filter($where);
    }
  }

}
