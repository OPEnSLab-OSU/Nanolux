import style from './style.css';
import Patterns from "../../components/patterns";
import NumericSlider from "../../components/numeric_slider";
import {useState, useEffect} from "preact/hooks";
import {getSettings, saveSettings} from "../../utils/api";
import LedCount from "../../components/led_count";
import {useModal} from "../../context/global_modal_context";
import {useConnectivity} from "../../context/online_context";

const Settings = () => {
	const { openModal } = useModal();
	const { isConnected } = useConnectivity();

	const [settings, setSettings] = useState({});

	useEffect(() => {
		if (isConnected) {
			getSettings().then(data => setSettings(data));
		}
	}, [isConnected])

	const handleNoiseChange = async (newValue) => {
		setSettings(current => ({...current, noise: newValue}));
		if (isConnected) {
			await saveSettings(settings);
		}
	}

	const handleLedChange = async (newValue) => {
		setSettings(current => ({...current, ledCount: newValue}));
		if (isConnected) {
			await saveSettings(settings);
		}
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
				<button
					className={style.settings_control}
					onClick={openModal}
				>Test modal</button>
			</div>
		</div>
	);
};

export default Settings;
