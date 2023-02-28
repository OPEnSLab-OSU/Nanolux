import { h } from 'preact';
import {useEffect, useState} from "preact-hooks";
import {getSettings, saveSettings} from "../../utils/api";
import style from "../sets/style.css";
import Patterns from "../../components/patterns";
import NumericSlider from "../../components/numeric_slider";
import LedCount from "../../components/led_count";
import ApiMonitor from "../../components/api_monitor";

const Wifi = () => {
	const [settings, setSettings] = useState({});

	useEffect(() => {
		getSettings().then(data => setSettings(data));
	}, [])

	const handle = async (newValue) => {
		setSettings(current => ({...current, noise: newValue}));
		await saveSettings(settings);
	}

	const handleLedChange = async (newValue) => {
		setSettings(current => ({...current, ledCount: newValue}));
		await saveSettings(settings);
	}

	return (
		<div className={style.home}>
			<div className={style.settings_control}>
				<Patterns />
			</div>
			<div className={style.settings_control}>
				{ settings && <NumericSlider
					className={style.settings_control}
					label="Noise Threshold"
					savedValue={settings.noise}
					min={0}
					max={100}
					onValueChanged={handleNoiseChange}
				/> }
			</div>
			<dev className={style.settings_control}>
				<LedCount
					className={style.settings_control}
					label="Led Count"
					savedValue={settings.ledCount}
					min={1}
					max={100}
					onValueChanged={handleLedChange}
				/>
			</dev>
			<div className={style.settings_control}>
				<ApiMonitor
					className={style.settings_control}
				/>
			</div>
		</div>
	);
};

export default Wifi;
