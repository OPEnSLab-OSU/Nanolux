import style from './style.css';
import Patterns from "../../components/patterns";
import SplittingButton from "../../components/splitting_button";
import SecondaryPatterns from "../../components/secondary_patterns";
import NumericSlider from "../../components/numeric_slider";
import {useState, useEffect} from "preact/hooks";
import {getNoise, saveNoise} from "../../utils/api";
import {useConnectivity} from "../../context/online_context";

const Settings = () => {
	const { isConnected } = useConnectivity();

	const [settings, setSettings] = useState({});

	useEffect(() => {
		if (isConnected) {
			getNoise().then(data => setSettings(data));
		}
	}, [isConnected])

	const handleNoiseChange = async (newValue) => {
		setSettings(current => ({...current, noise: newValue}));
		if (isConnected) {
			await saveNoise(settings.noise);
		}
	}

	return (
		<div className={style.home}>
			<div className={style.settings_control}>
				<Patterns />
			</div>
			<div className={style.settings_control}>
				<SecondaryPatterns />
			</div>
			<div className={style.settings_control}>
				<SplittingButton />
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
		</div>
	);
};

export default Settings;
