import style from './style.css';
import Patterns from "../../components/patterns";
import NumericSlider from "../../components/numeric_slider";
import {useState, useEffect} from "preact/hooks";
import {getSettings} from "../../utils/api";
import LedCount from "../../components/led_count";
import ApiMonitor from "../../components/api_monitor";

const Settings = () => {
	const [settings, setSettings] = useState({});

	useEffect(() => {
		getSettings().then(data => setSettings(data));
	}, [])

	return (
		<div className={style.home}>
			<div className={style.settings_control}>
				<Patterns
					patterns={["Pattern1", "Pattern2", "Pattern3"]} />
			</div>
			<div className={style.settings_control}>
				<NumericSlider
					className={style.settings_control}
					label="Noise Threshold"
					savedValue={settings.noise_gate}
					min={0}
					max={100}
				/>
			</div>
			<dev className={style.settings_control}>
				<LedCount
					className={style.settings_control}
					label="Led Count"
					savedValue={settings.led_count}
					min={1}
					max={100}
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

export default Settings;
