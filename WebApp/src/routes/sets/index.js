import style from './style.css';
import Patterns from "../../components/patterns";
import ModeSelector from "../../components/mode_selector";
import SecondaryPatterns from "../../components/secondary_patterns";
import NumericSlider from "../../components/numeric_slider";
import Save_Entry from '../../components/save_entry';
import {useState, useEffect} from "preact/hooks";
import {getNoise, saveNoise, getAlpha, saveAlpha, 
	getSmoothing, saveSmoothing, getBrightness, saveBrightness,
	saveLength,
	getLength,
	saveMs,
	getMs} from "../../utils/api";
import {useConnectivity} from "../../context/online_context";
import useInterval from "../../utils/use_interval";
import DebugSelector from '../../components/debug_selector';

const Settings = () => {

	return (
		<tr >
			<th>
				<div className={style.home}>
					<div className={style.settings_control}>
						<Patterns />
					</div>
					<div className={style.settings_control}>
						{ <NumericSlider
							className={style.settings_control}
							label="Noise Threshold"
							min={0}
							max={100}
							getterFunction={getNoise}
							saveFunction={saveNoise}
							api_key="noise"
						/> }
					</div>
					<div className={style.settings_control}>
						{ <NumericSlider
							className={style.settings_control}
							label="Brightness"
							min={0}
							max={255}
							getterFunction={getBrightness}
							saveFunction={saveBrightness}
							api_key='brightness'
						/> }
					</div>
					<div className={style.settings_control}>
						{ <NumericSlider
							className={style.settings_control}
							label="Smoothing"
							min={0}
							max={175}
							getterFunction={getSmoothing}
							saveFunction={saveSmoothing}
							api_key='smoothing'
						/> }
					</div>
					<div className={style.settings_control}>
						{ <NumericSlider
							className={style.settings_control}
							label="LED Count"
							min={30}
							max={200}
							getterFunction={getLength}
							saveFunction={saveLength}
							api_key='len'
						/> }
					</div>
					<div className={style.settings_control}>
						{ <NumericSlider
							className={style.settings_control}
							label="Refresh Rate (milliseconds)"
							min={15}
							max={255}
							getterFunction={getMs}
							saveFunction={saveMs}
							api_key='ms'
						/> }
					</div>
					<div className={style.settings_control}>
						<ModeSelector />
					</div>
					<div className={style.settings_control}>
						<SecondaryPatterns />
					</div>	
					<div className={style.settings_control}>
						{ <NumericSlider
							className={style.settings_control}
							label="Z-Layering Transparency"
							min={0}
							max={255}
							getterFunction={getAlpha}
							saveFunction={saveAlpha}
							api_key='alpha'
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
					<div>
						<DebugSelector/>
					</div>
				</th>
			</tr>
	);
};

export default Settings;
