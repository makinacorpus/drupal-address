<?php

/**
 * @file
 * Implement an address field.
 */

/**
 * Implements hook_field_info().
 */
function address_field_info() {
  return array(
    'address' => array(
      'label' => t('Address'),
      'description' => t('This field stores relative data about an address.'),
      'settings' => array(),
      'instance_settings' => array(),
      'default_widget' => 'address',
      'default_formatter' => 'address',
    ),
  );
}

/**
 * Implements hook_field_is_empty().
 */
function address_field_is_empty($item, $field) {
  return empty($item['city']) || empty($item['postal_code']);
}

/**
 * Implements hook_field_presave().
 */
function address_field_presave($entity_type, $entity, $field, $instance, $langcode, &$items) {

  foreach ($items as &$item) {

    // Sanitize city name.
    $item['city_sanitize'] = address_normalize_city($item['city']);
    // Extract department code.
    $item['dept_code'] = address_extract_dept_code($item['postal_code']);
    // SQL field storage force column to be inserted by passing NULL.
    $item['insee_code'] = !empty($item['insee_code']) ? $item['insee_code'] : 0;
  }
}

/**
 * Implements hook_field_widget_info().
 */
function address_field_widget_info() {
  return array(
    'address' => array(
      'label' => t('Address'),
      'field types' => array('address'),
      'settings' => array(),
    ),
  );
}

/**
 * Implements hook_field_widget_form().
 */
function address_field_widget_form(&$form, &$form_state, $field, $instance, $langcode, $items, $delta, $element) {

  $element += array(
    '#type' => 'fieldset',
    '#title' => t('Address'),
    '#collapsible' => FALSE,
    '#element_validate' => array('address_element_validate'),
  );
  $element['thoroughfare'] = array(
    '#type' => 'textfield',
    '#title' => t('Thoroughfare'),
    '#default_value' => isset($items[$delta]['thoroughfare']) ? $items[$delta]['thoroughfare'] : '',
    '#maxlength' => 255,
  );
  $element['premise'] = array(
    '#type' => 'textfield',
    '#title' => t('Premise'),
    '#default_value' => isset($items[$delta]['premise']) ? $items[$delta]['premise'] : '',
    '#maxlength' => 255,
  );
  $element['sub_premise'] = array(
    '#type' => 'textfield',
    '#title' => t('Sub-premise'),
    '#default_value' => isset($items[$delta]['sub_premise']) ? $items[$delta]['sub_premise'] : '',
    '#maxlength' => 255,
  );
  $element['city'] = array(
    '#type' => 'textfield',
    '#title' => t('City'),
    '#default_value' => isset($items[$delta]['city']) ? $items[$delta]['city'] : '',
    '#maxlength' => 255,
    '#pre_render' => array('address_element_required'),
  );
  $element['postal_code'] = array(
    '#type' => 'textfield',
    '#title' => t('Postal code'),
    '#default_value' => isset($items[$delta]['postal_code']) ? $items[$delta]['postal_code'] : '',
    '#maxlength' => 5,
    '#size' => 5,
    '#element_validate' => array('address_element_validate_postal_code'),
    '#pre_render' => array('address_element_required'),
  );

  return $element;
}

/**
 * Pre-render element callback to theme required element.
 */
function address_element_required($element) {
  $element['#required'] = TRUE;
  return $element;
}

/**
 * Address element validate callback.
 */
function address_element_validate($element, &$form_state, $form) {

  foreach (element_children($element) as $child) {

    // If at least one child is not empty, postal_code and city must exists.
    if (!in_array($child, array('city', 'postal_code', '_weight'), TRUE) && !empty($element[$child]['#value'])
      && (empty($element['city']['#value']) || empty($element['postal_code']['#value']))) {

      if (empty($element['city']['#value'])) {
        form_error($element['city'], t('%name is required.', array('%name' => $element['city']['#title'])));
      }
      if (empty($element['postal_code']['#value'])) {
        form_error($element['postal_code'], t('%name is required', array('%name' => $element['postal_code']['#title'])));
      }
      break;
    }
  }
}

/**
 * Element validate callback.
 */
function address_element_validate_postal_code($element, &$form_state, $form) {
  $value = $element['#value'];
  if ($value != '' && (!is_numeric($value) || strlen($value) !== 5)) {
    form_error($element, t('%name must be a five digit number.', array('%name' => $element['#title'])));
  }
}

/**
 * Implements hook_field_formatter_info().
 */
function address_field_formatter_info() {
  $formatters = array(
    'address' => array(
      'label' => t('Address'),
      'field types' => array('address'),
      'settings' => array(),
    ),
  );

  return $formatters;
}

/**
 * Implements hook_field_formatter_view().
 */
function address_field_formatter_view($entity_type, $entity, $field, $instance, $langcode, $items, $display) {
  $element = array();

  foreach ($items as $delta => $item) {
    $element[$delta] = array(
      '#theme' => 'address_formatter',
      '#item' => $item,
      '#entity' => $entity,
      '#entity_type' => $entity_type,
    );
  }

  return $element;
}

/**
 * Returns HTML for a default address field formatter.
 *
 * @ingroup themeable
 */
function theme_address_formatter($variables) {
  $item = $variables['item'];
  $output = '';

  $output .= '<div class="address">';
  foreach (array('thoroughfare', 'premise', 'sub_premise', 'city', 'postal_code') as $element) {
    if (!empty($item[$element])) {
      $output .= '<div class="' . $element . '">' . $item[$element] . '</div>';
    }
  }
  $output .= '</div>';

  return $output;
}
