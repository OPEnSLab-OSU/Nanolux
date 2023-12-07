import style from './style.css';
import Patterns from "../../components/patterns";
import ModeSelector from "../../components/mode_selector";
import SecondaryPatterns from "../../components/secondary_patterns";
import NumericSlider from "../../components/numeric_slider";
import Save_Entry from '../../components/save_entry';
import {useState, useEffect} from "preact/hooks";
import {getNoise, saveNoise, getAlpha, saveAlpha} from "../../utils/api";
import {useConnectivity} from "../../context/online_context";
import useInterval from "../../utils/use_interval";

const Settings = () => {
	const { isConnected } = useConnectivity();

	const [settings, setSettings] = useState({});

	useEffect(() => {
		if (isConnected) {
			getNoise().then(data => setSettings(data));
			getAlpha().then(data => setSettings(data));
		}
	}, [isConnected])

	const handleNoiseChange = async (newValue) => {
		setSettings(current => ({...current, noise: newValue}));
		if (isConnected) {
			await saveNoise(settings.noise);
		}
	}

	const handleAlphaChange = async (newValue) => {
		setSettings(current => ({...current, alpha: newValue}));
		if (isConnected) {
			await saveAlpha(settings.alpha);
		}
	}

	useInterval(() => {
        if (isConnected) {
            refresh();
        }
    }, 1000);

    const refresh = () => {
        getAlpha().then(data => setSettings(data));
		getNoise().then(data => setSettings(data));
    }

	return (
		<tr >
			<th>
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
					<div className={style.settings_control}>
						<ModeSelector />
					</div>
					<div className={style.settings_control}>
						<SecondaryPatterns />
					</div>	
					<div className={style.settings_control}>
						{ settings && <NumericSlider
							className={style.settings_control}
							label="Z-Layering Transparency"
							savedValue={settings.alpha}
							min={0}
							max={255}
							onValueChanged={handleAlphaChange}
						/> }
					</div>
				</div>
				</th>
				<th >
					
					<div className={style.background0}>
						<Save_Entry 
							name="Default Pattern"
							idx='0'
						/>
					</div>
					<br></br>
					<div className={style.background1}>
						<Save_Entry 
							name="Saved Pattern 1"
							idx='1'
						/>
					</div>
					<div className={style.background2}>
						<Save_Entry 
							name="Saved Pattern 2"
							idx='2'
						/>
					</div>
					<div className={style.background3}>
						<Save_Entry 
							name="Saved Pattern 3"
							idx='3'
						/>
					</div>
					<div className={style.background4}>
						<Save_Entry 
							name="Saved Pattern 4"
							idx='4'
						/>
					</div>

				</th>
			</tr>
	);
};

export default Settings;
