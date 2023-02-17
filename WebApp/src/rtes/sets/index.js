import style from './style.css';
import Patterns from "../../components/patterns";
import NumericSlider from "../../components/numeric_slider";
import SimpleSlider from "../../components/simple_slider";
import {useState, useEffect} from "preact/hooks";
import {getSettings} from "../../utils/api";

const Settings = () => {
	const [settings, setSettings] = useState({});

	useEffect(() => {
		getSettings().then(data => setSettings(data));
	}, [])

	return (
		<div className={style.home}>
			<Patterns patterns={["Pattern1", "Pattern2", "Pattern3"]} />
			<NumericSlider
				label="Noise Threshold"
				savedValue={settings.noise_gate}
				min={0}
				max={100}
			/>
			<NumericSlider label="Compression threshold" />
			<SimpleSlider label="Low Frequency Color" />
			<SimpleSlider label="High Frequency Color" />
		</div>
	);
};

export default Settings;
