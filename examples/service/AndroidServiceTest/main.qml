import QtQuick 2.11
import QtQuick.Layouts 1.11
import QtQuick.Window 2.11
import QtQuick.Controls 2.4
import de.skycoder42.QtService 1.2

Window {
	visible: true
	width: 640
	height: 480
	title: qsTr("Hello World")

	Connections {
		target: control
		onErrorChanged: {
			console.log(error);
			errorDialog.open();
		}
	}

	Dialog {
		id: errorDialog
		title: "Service error"
		standardButtons: Dialog.Ok
		visible: false
		Label {
			anchors.fill: parent
			text: control.error
		}
	}

	GridLayout {
		anchors.fill: parent
		columns: 2

		Label {
			Layout.columnSpan: 2
			Layout.fillWidth: true
			text: "Service valid: " + control.serviceExists()
		}

		Button {
			text: "Start Service"
			Layout.fillWidth: true
			onClicked: control.start();
		}

		Button {
			text: "Stop Service"
			Layout.fillWidth: true
			onClicked: control.stop();
		}

		Button {
			text: "Bind Service"
			Layout.fillWidth: true
			onClicked: helper.bind();
		}

		Button {
			text: "Unbind Service"
			Layout.fillWidth: true
			onClicked: helper.unbind();
		}

		Button {
			text: "Start with intent"
			Layout.fillWidth: true
			onClicked: helper.startIntent(actionField.text)
		}

		TextField {
			id: actionField
			placeholderText: "start action text"
			Layout.fillWidth: true
		}
	}
}
