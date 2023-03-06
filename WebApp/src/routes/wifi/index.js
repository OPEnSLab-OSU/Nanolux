import { h } from 'preact';
import {joinWiFi} from "../../utils/api";
import style from "./style.css";
import WifiSelector from "../../components/wifi_selector";
import {useState} from "preact/hooks";
// import NumericSlider from "../../components/numeric_slider";
// import LedCount from "../../components/led_count";
// import ApiMonitor from "../../components/api_monitor";

const Wifi = () => {
	// const [settings, setSettings] = useState({});
	const [currentWifi, setCurrentWifi] = useState(null);
	const [locked, setLocked] = useState(false)

	// useEffect(() => {
	// 	getSettings().then(data => setSettings(data));
	// }, [])

	const handleNetworkSelected = async (newWifi) => {
		setCurrentWifi(newWifi);
		// await joinWiFi(newWifi);
		if (newWifi) {
			setLocked(newWifi.lock);
		}
	}

	// const handleLedChange = async (newValue) => {
	// 	setSettings(current => ({...current, ledCount: newValue}));
	// 	await saveSettings(settings);
	// }

	return (
		<div className={style.home}>
			<div>Available Networks</div>
			<div className={style.settings_control}>
				<WifiSelector placeholder="Select a network..." onNetworkSelected={handleNetworkSelected} />
			</div>
			{ locked &&
				<div>
					Ask for password.
				</div>
			}
			{/*<div className={style.settings_control}>*/}
			{/*	{ settings && <NumericSlider*/}
			{/*		className={style.settings_control}*/}
			{/*		label="Noise Threshold"*/}
			{/*		savedValue={settings.noise}*/}
			{/*		min={0}*/}
			{/*		max={100}*/}
			{/*	/> }*/}
			{/*</div>*/}
			{/*<div className={style.settings_control}>*/}
			{/*	<LedCount*/}
			{/*		className={style.settings_control}*/}
			{/*		label="Led Count"*/}
			{/*		savedValue={settings.ledCount}*/}
			{/*		min={1}*/}
			{/*		max={100}*/}
			{/*		onValueChanged={handleLedChange}*/}
			{/*	/>*/}
			{/*</div>*/}
			{/*<div className={style.settings_control}>*/}
			{/*	<ApiMonitor*/}
			{/*		className={style.settings_control}*/}
			{/*	/>*/}
			{/*</div>*/}
		</div>
	);
};

export default Wifi;
