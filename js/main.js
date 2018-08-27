$(function () {

	var soundInstances = {};

	$('.player .button').on('click', function (e) {
		e.preventDefault();

		var $this   = $(this);
		var $parent = $this.parents('.player');
		var href    = $this.attr('href');
		var volume  = parseFloat($parent.data('volume') || 0.5);
		var soundid = $parent.data('soundid');
		var state   = $parent.data('state') || '';
		var instance;

		if (state == 'playing') {
			instance = soundInstances[soundid];
			instance.stop();
			complete();
			return;
		}
		else if (state != '') {
			return;
		}

		function loading() {
			$parent.data('state', 'loading');
			$parent.addClass('loading');
		}

		function play() {
			$parent.data('state', 'playing');
			$parent.removeClass('loading').addClass('playing');
		}

		function complete() {
			$parent.data('state', '');
			$parent.removeClass('playing');
		}

		function error() {
			$parent.data('state', 'error');
			$parent.removeClass('loading').addClass('error');
		}

		if (!soundid) {
			soundid = 'sound' + (new Date()).getTime() + parseInt(Math.random() * 1000);
			$parent.data('soundid', soundid);
		}

		if (soundInstances[soundid]) {
			instance = soundInstances[soundid];
			instance.play('sound', {volume: volume});
			play();
		}
		else {
			var queue = new createjs.LoadQueue();
			queue.installPlugin(createjs.Sound);

			queue.addEventListener('fileload', function(e) {
				instance = createjs.Sound.play(soundid, {volume: volume});
				soundInstances[soundid] = instance;

				instance.addEventListener('complete', function(e) {
					complete();
				});

				play();
			});

			queue.addEventListener('error', function(e) {
				error();
			});

			queue.loadManifest([
				{id: soundid, src: href}
			], true);

			loading();
		}
	});

});
