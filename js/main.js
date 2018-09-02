(function () {

	var players = document.querySelectorAll('.player');

	[].slice.call(players).forEach(function (player) {
		var button = player.querySelector('.button');
		var audio = document.createElement('audio');

		audio.preload = 'none';
		audio.src = button.href;

		button.addEventListener('click', function (e) {
			if (audio.paused) {
				audio.currentTime = 0;
				audio.play();
				player.classList.add('loading');
			}
			else {
				audio.pause();
			}

			e.preventDefault();
		});

		audio.addEventListener('playing', function () {
			player.classList.remove('loading');
			player.classList.add('playing');
		});

		audio.addEventListener('pause', function () {
			player.classList.remove('playing', 'loading');
		});
	});

}());
