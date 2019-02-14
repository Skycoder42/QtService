package de.skycoder42.qtservice;

import java.util.concurrent.Semaphore;

import android.content.Intent;

import org.qtproject.qt5.android.bindings.QtService;

public class AndroidService extends QtService {
	private final Semaphore _startSem = new Semaphore(0);
	private final Semaphore _exitSem = new Semaphore(0);

	private static native int callStartCommand(Intent intent, int flags, int startId, int oldCode);
	private static native boolean exitService();

	public void nativeReady() {
		_startSem.release();
	}

	public void nativeExited() {
		_exitSem.release();
	}

	@Override
	public int onStartCommand(Intent intent, int flags, int startId) {
		int res = super.onStartCommand(intent, flags, startId);
		try {
			_startSem.acquire();
		} catch(InterruptedException e) {
			e.printStackTrace();
			return res;
		}
		res = callStartCommand(intent, flags, startId, res);
		_startSem.release();
		return res;
	}

	@Override
	public void onDestroy() {
		try {
			if(exitService())
				_exitSem.acquire();
		} catch(InterruptedException e) {
			e.printStackTrace();
		}
		_exitSem.release();
		super.onDestroy();
	}
}
