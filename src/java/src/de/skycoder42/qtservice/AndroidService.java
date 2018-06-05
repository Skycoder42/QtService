package de.skycoder42.qtservice;

import java.util.concurrent.Semaphore;

import android.content.Intent;

import org.qtproject.qt5.android.bindings.QtService;

public class AndroidService extends QtService {
	private int _exitCode = 0;
	private final Semaphore _sem = new Semaphore(0);

	private static native int callStartCommand(Intent intent, int flags, int startId, int oldCode);

	private void nativeReady() {
		_sem.release();
	}

	@Override
	public void onDestroy() {
		super.onDestroy();
		// explicitly exit to prevent the process from beeing cached
		System.exit(_exitCode);
	}

	@Override
	public int onStartCommand(Intent intent, int flags, int startId) {
		int res = super.onStartCommand(intent, flags, startId);
		try {
			_sem.acquire();
		} catch(InterruptedException e) {
			e.printStackTrace();
			return res;
		}
		res = callStartCommand(intent, flags, startId, res);
		_sem.release();
		return res;
	}

	public void stopSelfSecure() {
		stopService(new Intent(this, this.getClass()));//Stop myself
	}
}
