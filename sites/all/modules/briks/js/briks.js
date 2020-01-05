(function ($) {
      Drupal.behaviors.briks = {
        attach: function (context) {
          $('#briks-calc-form').submit(function () {
            let self = $(this);
            $.ajax({
              url: '/briks/ajax/',
              data: self.serialize(),
              method: 'POST',
              type: 'JSON',
              success: function (data) {
                console.log(data);
                let html = '<ul>';
                $.each(data.res, function (k, v) {
                  html += '<li><span class="label">' + v[0] + '</span><span class="value">' + v[1] + '</span><span class="ei">' + v[2] + '</span></li>';
                })
                html += '</ul>';
                $('#briks_calc_result').html(html);
              }
            })
            return false;
          })
        }
      }
    }
)(jQuery);