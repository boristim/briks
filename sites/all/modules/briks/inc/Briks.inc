<?php

class Briks {

  const STROY_CALC = 'https://stroy-calc.ru/calc/calc-kirpicha2';

  private static $sizesJsPrices = '';

  private static $response = '';

  private static $vals = [];

  private static $wall_width = 0;

  private static $row_height = 0;

  private static $apertures_area = 0;

  private static $apertures_grid = 0;

  public static function sizesOptions() {
    $sizes = [];
    self::$sizesJsPrices = 'function brick_prices(npp){';
    $npp = 0;
    foreach (Briks::sizes() as $label => $info) {
      $sizes[++$npp] = "$label ($info[length] X $info[width] X $info[height])";
      self::$sizesJsPrices .= "if(npp==$npp){return{price:$info[price],length:$info[length],width:$info[width],height:$info[height],weight:$info[weight]};}";
    }
    self::$sizesJsPrices .= '}';
    return $sizes;
  }

  public static function sizesJs() {
    if (!mb_strlen(self::$sizesJsPrices)) {
      self::sizesOptions();
    }
    $calcJs = <<<JS
(function ($) {
  $('#edit-sizes').change(function () {
    set_brick_params($(this).val())
  });  
  var set_brick_params = function(npp){
    let brick = brick_prices(npp);
    $('#edit-price').val(brick.price);
    $('#edit-weight').val(brick.weight);
    $('[name=b_height]').val(brick.height);
    $('[name=b_length]').val(brick.length);
    $('[name=b_width]').val(brick.width);
  };
  set_brick_params(1);
})(jQuery)
JS;

    return self::$sizesJsPrices . $calcJs;
  }

  private static function sizesPreconfigured() {
    $ret['Одинарный кирпич 1НФ'] = [
      'length' => 250,
      'width' => 120,
      'height' => 65,
      'price' => 10,
      'weight' => 3.6,
    ];
    $ret['Полуторный кирпич 1.4НФ'] = [
      'length' => 250,
      'width' => 120,
      'height' => 88,
      'price' => 14,
      'weight' => 3.6,
    ];
    $ret['Двойной кирпич 2.1НФ'] = [
      'length' => 250,
      'width' => 120,
      'height' => 138,
      'price' => 21,
      'weight' => 3.6,
    ];
    $ret['Евро кирпич 0.7НФ'] = [
      'length' => 250,
      'width' => 85,
      'height' => 65,
      'price' => 7,
      'weight' => 3.6,
    ];
    $ret['Половинчатый кирпич 0.5НФ'] = [
      'length' => 250,
      'width' => 60,
      'height' => 65,
      'price' => 5,
      'weight' => 3.6,
    ];
    $ret['Модульный одинарный'] = [
      'length' => 288,
      'width' => 138,
      'height' => 65,
      'price' => 12,
      'weight' => 3.6,
    ];
    $ret['Модульный утолщённый'] = [
      'length' => 288,
      'width' => 138,
      'height' => 88,
      'price' => 15,
      'weight' => 3.6,
    ];
    return $ret;
  }

  private static function sizesDatabse() {
    $ret = FALSE;
    $query = db_query("Select nid from {node} where type='brick' and status = 1");
    if ($query->rowCount()) {
      $ret = [];
      while ($row = $query->fetchAssoc()) {
        $brick = node_load($row['nid']);
        $ret[$brick->title] = [
          'length' => $brick->field_length[LANGUAGE_NONE][0]['value'],
          'width' => $brick->field_width[LANGUAGE_NONE][0]['value'],
          'height' => $brick->field_height[LANGUAGE_NONE][0]['value'],
          'weight' => $brick->field_weight[LANGUAGE_NONE][0]['value'],
          'price' => $brick->field_price[LANGUAGE_NONE][0]['value'],
        ];
      }

    }
    return $ret;
  }

  private static function sizes() {
    if (!$ret = self::sizesDatabse()) {
      $ret = self::sizesPreconfigured();
    }
    return $ret;
  }

  public static function wallThinknesOptions() {
    return [
      '0' => 'Половина кирпича',
      '1' => 'В 1 кирпич',
      '2' => 'В 1.5',
      '3' => 'В 2',
    ];
  }

  public static function seamThinknesOptions() {
    return [
      '1' => 'Раствор 10',
      '1.5' => 'Раствор 15',
      '2' => 'Раствор 20',
    ];
  }

  public static function wallGridThinknesOptions() {
    return [
      '1' => 'Каждый ряд',
      '2' => 'Через ряд',
      '3' => 'Через 2 ряда',
      '4' => 'Через 3 ряда',
      '5' => 'Через 4 ряда',
    ];
  }

  private static function parseResponse() {
    $res = [];
    if (preg_match_all('/<tr>(.*)<\/tr>/isU', self::$response, $trs, PREG_SET_ORDER)) {
      foreach ($trs as $row) {
        if (preg_match_all('/<td>(.*)<\/td>/isU', $row[1], $tds, PREG_SET_ORDER)) {
          $res_row = [];
          foreach ($tds as $td) {
            $res_row[] = strip_tags($td[1]);
          }
          $res[] = $res_row;
        }
      }
      unset($res[16]);
    }
    return $res;
  }

  private static function stroyCalc() {
    $params = [
      'mat' => 'r',
      'visotabloka' => self::$vals['b_height'],
      'shirinabloka' => self::$vals['b_width'],
      'dlinabloka' => self::$vals['b_length'],
      'vesb' => self::$vals['weight'],
      'dlinasten' => self::$vals['perimeter'],
      'visotasten' => self::$vals['height'],
      'tolshinasten' => self::$vals['wall_depth'],
      'tolshinakladki' => self::$vals['seam_depth'],
      'klr' => self::$vals['wall_grid'],
      'cena' => self::$vals['price'],
      'pust' => self::$vals['voidness'],
      'kfr' => self::$vals['gabel_count'],
      'vfr' => self::$vals['gabel_height'],
      'shfr' => self::$vals['gabel_width'],
      'ok_v' => self::$vals['window_height'],
      'ok_h' => self::$vals['window_width'],
      'ok_S' => self::$vals['window_count'],
      'dv_v' => self::$vals['door_height'],
      'dv_h' => self::$vals['door_width'],
      'dv_S' => self::$vals['door_count'],
      'ok_v2' => self::$vals['window_height_2'],
      'ok_h2' => self::$vals['window_width_2'],
      'ok_S2' => self::$vals['window_count_2'],
      'dv_v2' => self::$vals['door_height_2'],
      'dv_h2' => self::$vals['door_width_2'],
      'dv_S2' => self::$vals['door_count_2'],
    ];
    $ch = curl_init();
    curl_setopt_array($ch, [
      CURLOPT_URL => self::STROY_CALC,
      CURLOPT_POST => 1,
      CURLOPT_POSTFIELDS => ['u' => http_build_query($params)],
      CURLOPT_RETURNTRANSFER => TRUE,
      CURLOPT_SSL_VERIFYPEER => FALSE,
    ]);
    self::$response = curl_exec($ch);
    curl_close($ch);
    return self::parseResponse();
  }

  public static function emulateQuery($vals) {
    self::$vals = $vals;
    if ($vals['calc_switch'] == 1) {
      return self::myCalc();
    }
    else {
      return self::stroyCalc();
    }
  }

  private static function myCalc() {
    $ret = [];
    $seam_mass = 1900;
    $seam = self::$vals['seam_depth'] * 10;
    self::$apertures_area = 0;
    self::$row_height = $row_height = self::$vals['b_height'] + $seam;

    $rows = ceil((self::$vals['height'] * 10) / $row_height);

    $grid_length = $rows * self::$vals['perimeter'];

    $brick_area = self::$vals['b_length'] * self::$vals['b_width'];

    switch (self::$vals['wall_depth']) {
      case 3:
        $wall_width = 2 * (self::$vals['b_length'] + $seam);
        $seam_per_break_area = self::$vals['b_length'] * 1.7 * $seam;
        break;
      case 2:
        $wall_width = self::$vals['b_length'] + $seam + self::$vals['b_width'];
        $seam_per_break_area = self::$vals['b_length'] * (3.5 / 3) * $seam;
        break;
      case 1:
        $wall_width = self::$vals['b_length'];
        $seam_per_break_area = self::$vals['b_length'] * (0.9) * $seam;
        break;
      case 0:
      default:
        $wall_width = self::$vals['b_width'];
        $seam_per_break_area = self::$vals['b_width'] * (1.4) * $seam;
    }
    self::$wall_width = $wall_width;
    $total_area = (self::$vals['perimeter'] * 1000 * self::$vals['height'] * 10);
    $brick_volume = $brick_area * self::$vals['b_height'];
    $briks_total_volume = (self::$vals['perimeter'] * 1000 * $wall_width * self::$vals['height'] * 10);
    $total_briks = ceil($briks_total_volume / ($brick_volume + ($seam_per_break_area * self::$vals['b_height']) + ($brick_area * $seam)));
    $apertures_volume = self::calcApertures(self::$vals['window_width'] * 10, self::$vals['window_height'] * 10, self::$vals['window_count'])
      + self::calcApertures(self::$vals['door_width'] * 10, self::$vals['door_height'] * 10, self::$vals['door_count'])
      + self::calcApertures(self::$vals['window_width_2'] * 10, self::$vals['window_height_2'] * 10, self::$vals['window_count_2'])
      + self::calcApertures(self::$vals['door_width_2'] * 10, self::$vals['door_height_2'] * 10, self::$vals['door_count_2']);

    $grid_length -= self::$apertures_grid;

    $apertures_brick_count = ceil($apertures_volume / ($brick_volume + ($seam_per_break_area * self::$vals['b_height']) + ($brick_area * $seam)));
    $total_briks -= ceil($apertures_brick_count);
    $total_area -= self::$apertures_area;
    if ((self::$vals['gabel_count'] * self::$vals['gabel_height'] * self::$vals['gabel_width']) > 0) {

      $gabel_rows = self::$vals['gabel_height'] * 10 / self::$row_height;
      $gabel_grid = ceil(self::$vals['gabel_width'] * 10 * $gabel_rows / 2 / 1000);
      $gabel_area = self::$vals['gabel_count'] * 10 * self::$vals['gabel_height'] * 10 * self::$vals['gabel_width'] / 2;
      $gabels_volume = $gabel_area * self::$wall_width;
      $gabels_brick_count = floor($gabels_volume / ($brick_volume + $seam_per_break_area));
      $total_briks += $gabels_brick_count;
      $total_area -= ($gabel_area / 1000000);
      $grid_length += self::$vals['gabel_count'] * $gabel_grid;
    }


    $total_seam_volume = ($seam_per_break_area * self::$vals['b_height'] * $total_briks) + ($total_briks * $brick_area * $seam);

    $ret[] = ['Периметр строения', self::$vals['perimeter'], 'м.'];
    $ret[] = [
      'Общая площадь кладки',
      round($total_area / 1000000, 2),
      'м.кв.',
    ];
    $ret[] = ['Толщина стены', $wall_width, 'мм.'];
    $ret[] = ['Количество кирпича', $total_briks, 'шт.'];
    $ret[] = ['Вес 1 кирпича', self::$vals['weight'], 'кг.'];
    $ret[] = [
      'Общий вес кирпича',
      round(self::$vals['weight'] * $total_briks, 3),
      'кг.',
    ];
    $ret[] = [
      'Общий объем кирпича',
      round($brick_volume / 1000000000 * $total_briks, 2),
      'м.куб.',
    ];
    $ret[] = [
      'Общая стоимость кирпича',
      self::$vals['price'] * $total_briks,
      'руб.',
    ];
    $ret[] = [
      'Количество кирпича в кубе',
      round(1000000000 / $brick_volume, 2),
      'шт.',
    ];
    $ret[] = [
      'Раствора на всю кладку',
      round($total_seam_volume / 1000000000, 2),
      'м.куб.',
    ];
    $seam_total_mass = $total_seam_volume / 1000000000 * $seam_mass;
    $ret[] = [
      'Примерный вес раствора',
      round($seam_total_mass, 2),
      'кг.',
    ];
    $ret[] = [
      'Кол-во рядов кирпича с учетом швов',
      round($rows),
      '',
    ];
    $ret[] = [
      'Количество кладочной сетки',
      round($grid_length / self::$vals['wall_grid'], 2),
      'м.',
    ];
    $total_mass = (self::$vals['weight'] * $total_briks) + $seam_total_mass;
    $ret[] = [
      'Примерный вес готовых стен',
      round($total_mass, 2),
      'кг.',
    ];

    $ret[] = [
      'Нагрузка на фундамент от стен',
      $total_mass > 0 ? round(self::$vals['perimeter'] * 100 * $wall_width / $total_mass / 30, 2) : 0,
      'кг/см.кв.',
    ];
    return $ret;
  }

  private static function calcApertures($width, $height, $count) {

    if (($width * $width * $count) > 0) {
      self::$apertures_area += $width * $height * $count;
      self::$apertures_grid += ceil(($height / self::$row_height) * $width / 1000);
      return $width * $height * $count * self::$wall_width;
    }
    return 0;
  }

}