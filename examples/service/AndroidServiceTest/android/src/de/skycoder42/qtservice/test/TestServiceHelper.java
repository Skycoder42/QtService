package de.skycoder42.qtservice.test;

import android.content.Context;
import android.app.Service;
import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;

class TestServiceHelper
{
	private static final int NotId = 42;
	private static final String ChannelId = "42";

	public static void registerChannel(Context context)
	{
		NotificationChannel foreground = new NotificationChannel(ChannelId,
				"Test Service",
				NotificationManager.IMPORTANCE_MIN);
		foreground.setDescription("Test Service");
		foreground.enableLights(false);
		foreground.enableVibration(false);
		foreground.setShowBadge(false);

		NotificationManager manager = (NotificationManager) context.getSystemService(Context.NOTIFICATION_SERVICE);
		manager.createNotificationChannel(foreground);
	}

	public static void notifyRunning(Service context, String message)
	{
		Notification.Builder builder = new Notification.Builder(context, ChannelId)
			.setContentTitle("Test Service")
			.setContentText(message)
			.setSmallIcon(android.R.drawable.ic_media_play)
			.setOngoing(true);

		context.startForeground(NotId, builder.build());
	}

	public static void updateNotifyRunning(Service context, String message)
	{
		Notification.Builder builder = new Notification.Builder(context, ChannelId)
			.setContentTitle("Test Service")
			.setContentText(message)
			.setSmallIcon(android.R.drawable.ic_media_ff)
			.setOngoing(true);

		NotificationManager manager = (NotificationManager) context.getSystemService(Context.NOTIFICATION_SERVICE);
		manager.notify(NotId, builder.build());
	}

	public static void stopNotifyRunning(Service context)
	{
		context.stopForeground(true);
	}
}
