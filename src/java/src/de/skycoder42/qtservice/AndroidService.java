package de.skycoder42.qtservice;

import org.qtproject.qt5.android.bindings.QtService;

class AndroidService extends QtService {
	private int _exitCode = 0;

	@Override
	public void onDestroy() {
		super.onDestroy();
		// explicitly exit to prevent the process from beeing cached
		System.exit(_exitCode);
	}

	@Override
	public void stopSelf() {
		stopService(new Intent(this, this.getClass()));//Stop myself
	}
}
