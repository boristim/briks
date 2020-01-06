<?php

class Briks {

  const STROY_CALC = 'https://stroy-calc.ru/calc/calc-kirpicha2';

  private static $sizesJsPrices = '';

  private static $response = '';

  private static $vals = [];

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

  private static function sizes() {

    $query = db_query("Select nid from {node} where type='brick' and status = 1");
    if ($query->rowCount()) {
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
      return $ret;
    }

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
    if (preg_match_all('/<tr>(.*)<\/tr>/isU', self::$response, $matches, PREG_SET_ORDER)) {
      foreach ($matches as $row) {
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
    $vals = self::$vals;
    $params = [
      'mat' => 'r',
      'visotabloka' => $vals['b_height'],
      'shirinabloka' => $vals['b_width'],
      'dlinabloka' => $vals['b_length'],
      'vesb' => $vals['weight'],
      'dlinasten' => $vals['perimeter'],
      'visotasten' => $vals['height'],
      'tolshinasten' => $vals['wall_depth'],
      'tolshinakladki' => $vals['seam_depth'],
      'klr' => $vals['wall_grid'],
      'cena' => $vals['price'],
      'pust' => $vals['voidness'],
      'kfr' => 0,
      'vfr' => 0,
      'shfr' => 0,
      'ok_v' => 0,
      'ok_h' => 0,
      'ok_S' => 0,
      'dv_v' => 0,
      'dv_h' => 0,
      'dv_S' => 0,
      'ok_v2' => 0,
      'ok_h2' => 0,
      'ok_S2' => 0,
      'dv_v2' => 0,
      'dv_h2' => 0,
      'dv_S2' => 0,
    ];
    $post = ['u' => http_build_query($params)];
    $ch = curl_init();
    $params = [
      CURLOPT_URL => self::STROY_CALC,
      CURLOPT_POST => 1,
      CURLOPT_POSTFIELDS => $post,
      CURLOPT_RETURNTRANSFER => TRUE,
      CURLOPT_SSL_VERIFYPEER => FALSE,
    ];
    curl_setopt_array($ch, $params);
    self::$response = curl_exec($ch);
    curl_close($ch);
    return self::parseResponse();
  }

  public static function emulateQuery($vals) {
    self::$vals = $vals;
    return self::myCalc();
    // OR STROY CALC
//    return self::stroyCalc();
  }

  private static function myCalc() {
    $ret = [];
    $seam = self::$vals['seam_depth'] * 10;

    $row_height = (self::$vals['b_height']) + $seam;
    $rows = ceil((self::$vals['height'] * 10) / $row_height);

    $brick_area = self::$vals['b_length'] * self::$vals['b_width'];

    switch (self::$vals['wall_depth']) {
      case 3:
        $wall_width = 2 * (self::$vals['b_length'] + $seam);
        break;
      case 2:
        $wall_width = self::$vals['b_length'] + $seam + self::$vals['b_width'];
        break;
      case 1:
        $wall_width = self::$vals['b_length'];
        break;
      case 0:
      default:
        $wall_width = self::$vals['b_width'];
    }
    $seam_koeff = 1.3;
    $seam_mass = 1900;
    $corner_area = ($wall_width ^ 2) / 2;
    $per_row = (self::$vals['perimeter'] * 1000 * $wall_width) / ($brick_area + ($seam * $wall_width));
    $brick_volume = $brick_area * self::$vals['b_height'];
    $per_corner = $corner_area / $brick_area;
    $total_briks = floor(($rows * $per_row) - ($per_corner * 4));
    $seam_volume = $brick_area * $seam;
    $total_seam = $seam_volume * $total_briks * $seam_koeff;
    $ret[] = ['Периметр строения', self::$vals['perimeter'], 'м.'];
    $ret[] = [
      'Общая площадь кладки',
      self::$vals['perimeter'] * self::$vals['height'] / 100,
      'м.кв.',
    ];
    $ret[] = ['Толщина стены', $wall_width, 'мм.'];
    $ret[] = ['Количество кирпича', $total_briks, 'шт.'];
    $ret[] = ['Вес 1 кирпича', self::$vals['weight'], 'кг.'];
    $ret[] = ['Общий вес кирпича', self::$vals['weight'] * $total_briks, 'кг.'];
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
      round($total_seam / 1000000000, 2),
      'м.куб.',
    ];
    $seam_total_mass = $total_seam / 1000000000 * $seam_mass;
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
      round($rows * self::$vals['perimeter'] / self::$vals['wall_grid'], 2),
      'шт.',
    ];
    $total_mass = (self::$vals['weight'] * $total_briks) + $seam_total_mass;
    $ret[] = [
      'Примерный вес готовых стен',
      round($total_mass, 2),
      'кг.',
    ];

    $ret[] = [
      'Нагрузка на фундамент от стен',
      round(self::$vals['perimeter'] * 100 * $wall_width / $total_mass / 30, 2),
      'кг/см.кв.',
    ];
    return $ret;
  }
}