<?php

/**
 * @file
 * The class for "Publication by Type" type widgets.
 */

class os_boxes_pub_type extends os_boxes_default {

  public $embed_as = array(
    'iframe',
  );

  /**
   * Implementation of boxes_content::options_defaults().
   */
  public function options_defaults() {
    $options = parent::options_defaults();

    $options['whitelist'] = array('all' => 'all');
    $options['display_num'] = 1;

    return $options;
  }

  /**
   * Implementation of boxes_content::options_form().
   */
  public function options_form(&$form_state){
    $rows = db_select('biblio_types', 'bt')
            ->fields('bt', array('tid', 'name'))
            ->condition('tid', 0, '>')
            ->condition('visible', 1)
            ->execute();
    $options = array('all' => 'Select All');
    $value = $this->options['whitelist'];
    $all = in_array('all', $value) && $value['all'] === 'all';
    foreach ($rows as $r) {
      $options[$r->tid] = $r->name;
      if ($all) $value[$r->tid] = $r->tid;
    }

    $form['whitelist'] = array(
      '#type' => 'checkboxes',
      '#options' => $options,
      '#default_value' => $value,
      '#title' => t('Show the following types:'),
      '#attributes' => array('class' => array('pub_type')),
      '#description' => t('Select the types you want to appear in the widget.'),
      '#attached' => array(
        'js' => array(
          drupal_get_path('module', 'os_boxes').'/misc/selectAll.js',
        ),
      ),
    );

    $form['display_num'] = array(
      '#type' => 'checkbox',
      '#title' => t('Display number of items in category'),
      '#default_value' => $this->options['display_num'],
    );

    $form += parent::options_form($form_state);
    return $form;
  }

  /**
   * Implementation of boxes_content::render().
   */
  public function render() {
    $block = parent::render();
    $this->set_cache_id('biblio');
    drupal_add_library('system', 'drupal.ajax');
    if (($block['content'] = $this->get_cache()) === FALSE) {
      $block['content'] = os_publications_sort_publication_block('type', 'types', $this->delta);

      $this->set_cache($block['content']);
    }
    return $block;
  }

  /**
   * Called from views_pre_view
   *
   * @param view $view
   */
  public function views_alter(&$view, &$display_id, &$args){
  }

  /**
   * Return the cache parameters for this box
   */
  public function cache_info() {
    // Caching disabled so ajax pager gets init'ed.
  	return array();
  }

}
