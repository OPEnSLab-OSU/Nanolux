import style from './style.css';
import Patterns from "../../components/patterns";
import NumericSlider from "../../components/numeric_slider";
import {useState, useEffect} from "preact/hooks";
import {getSettings} from "../../utils/api";
import LedCount from "../../components/led_count";

const Settings = () => {
	const [settings, setSettings] = useState({});

	useEffect(() => {
		getSettings().then(data => setSettings(data));
	}, [])

	return (
		<div className={style.home}>
			<section>
				<Patterns
					className={style.settings_control}
					patterns={["Pattern1", "Pattern2", "Pattern3"]} />
			</section>
			<section>
				<NumericSlider
					className={style.settings_control}
					label="Noise Threshold"
					savedValue={settings.noise_gate}
					min={0}
					max={100}
				/>
			</section>
			<section>
				<LedCount
					className={style.settings_control}
					label="Led Count"
					savedValue={settings.led_count}
					min={1}
					max={100}
				/>
			</section>
		</div>
	);
};

export default Settings;
