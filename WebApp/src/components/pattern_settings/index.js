import Patterns from "../../components/patterns";
import NumericSlider from "../../components/numeric_slider";
import {useState, useEffect} from "preact/hooks";
import {
	getPattern,
	updatePattern} from "../../utils/api";
import {useConnectivity} from "../../context/online_context";
import useInterval from "../../utils/use_interval";
import { LabelSpinner } from '../../components/spinner';
import RANGE_CONSTANTS from '../../utils/constants';
import style from './style.css';
import MultiRangeSliderWrapper from '../../components/multi_range_slider';
import ConfigDropDown from "../config_drop_down";

/**
 * @brief An object meant to hold and display settings for a specific pattern
 * @param num 	The ID of the pattern to display
 * @param patterns	A list of patterns and their IDs
 */
const PatternSettings = ({num, patterns, advanced = false}) => {

	// Checks if the web app is connected to the device.
	const { isConnected } = useConnectivity();

	// Flag representing if initial data has been obtained from
	// the device.
	const [loading, setLoading] = useState(true);

	// Flag that tells the object to update the NanoLux device with new data.
	const [updated, setUpdated] = useState(false);

	// Pattern-level data structure
	const [data, setData] = useState({
		idx: 0,
		hue_max: 255,
		hue_min: 0,
		brightness: 255,
		smoothing: 0,
		postprocess: 0,
		config: 0
	});

	/**
	 * @brief Manages initial querying of the data from the NanoLux device.
	 * 		  Sets the loading flag to false when done.
	 */
	useEffect(() => {
        if (isConnected) {
            getPattern(num)
			.then(data => {setData(data)})
			.then(setLoading(false));
        }  
    }, [isConnected])

	/**
	 * @brief Updates the pattern on the Nanolux device 
	 * if it is connected and has modified data,
	 * then flags the data structure.
	 */
	useInterval(() => {
        if (isConnected && updated) {
            updatePattern(num, data);
			setUpdated(false);
        }
    }, 100);

	/**
	 * @brief Continuously poll the Nanolux device for changes to the
	 * hardware pattern index, updates the web app UI to reflect the change.
	 */
	useInterval(() => {
		if (isConnected && !updated) {
			getPattern(num).then(newData => {
				//console.log("Fetched pattern from ESP32:", newData.idx);

				if (newData.idx !== data.idx) {
					//console.log("Updating UI with new pattern:", newData.idx)
					setData(newData);
				}
			});
		}
	}, 200);

	/**
	 * @brief Updates a parameter in the pattern data structure with a new value.
	 * @param ref The string reference to update in the data structure
	 * @param value The new value to update the data structure with
	 */
	const update = (ref, value) => {
		if(!loading){		
			setData((oldData) => {
				let newData = Object.assign({}, oldData);
				newData[ref] = value;
				return newData;
			})
		}
		
		setUpdated(true);
	}

	/**
	 * @brief Generates the Subpattern UI element itself. If initial data is not
	 * loaded, show a spinner instead.
	 */
	return (
		(!loading ? 
		<div>
			<br/>
			<Patterns
				initialID={data.idx}
				structure_ref={"idx"}
				update={update}
				patterns={patterns}
			/>
			<br/>
			{advanced && (
				<>
				<ConfigDropDown
					patternIdx={data.idx}
					structureRef={"config"}
					update={update}
					initial={data.config}
				/>
				<br/>
				</>
			)}
			<NumericSlider
				className={style.settings_control}
				label="Brightness"
				tooltip={{
					id: 'brightness',
					content: 'This slider adjusts how bright the LED strip is.',
				}}
				min={RANGE_CONSTANTS.BRIGHTNESS_MIN}
				max={RANGE_CONSTANTS.BRIGHTNESS_MAX}
				initial={data.brightness}
				structure_ref="brightness"
				update={update}
			/>
			<br/>
			<NumericSlider
				className={style.settings_control}
				label="Smoothing"
				tooltip={{
					id: 'brightness',
					content: 'This slider adjusts the smoothing of the LED strip.',
				}}
				min={RANGE_CONSTANTS.SMOOTHING_MIN}
				max={RANGE_CONSTANTS.SMOOTHING_MAX}
				initial={data.smoothing}
				structure_ref="smoothing"
				update={update}
			/>
			<br/>
			<div className={style.settings_control}>
                <label className={style.checkboxOption}>
					<input 
						type="checkbox" 
						id="reverse" 
						name="reverse" 
						checked={(data.postprocess == 1) || (data.postprocess == 3)}
						onChange={() => {
							switch (data.postprocess) {
								case 0:
									update("postprocess", 1);
									break;
								
								case 1:
									update("postprocess", 0);
									break;
								
								case 2:
									update("postprocess", 3);
									break;
							
								default:
									update("postprocess", 2);
									break;
							}
						}}
					/>
					Reverse
				</label>
				<label className={style.checkboxOption}>
					<input 
						type="checkbox" 
						id="mirror" 
						name="mirror" 
						checked={(data.postprocess == 2) || (data.postprocess == 3)}
						onChange={() => {
							switch (data.postprocess) {
								case 0:
									update("postprocess", 2);
									break;
								
								case 1:
									update("postprocess", 3);
									break;
								
								case 2:
									update("postprocess", 0);
									break;
							
								default:
									update("postprocess", 1);
									break;
							}
						}}
					/>
					Mirror
				</label>
            </div>
			<br></br>
			{advanced && (
				<>
				<MultiRangeSliderWrapper
					tooltip={{
						id: 'color',
						content: 'Adjusts the range of colors that are displayed on the LED strip.'
					}}
					min={0}
					max={255}
					selectedLow={data.hue_min}
					selectedHigh={data.hue_max}
					minRef={"hue_min"}
					maxRef={"hue_max"}
					update={update}
				/>
				</>
			)}
			<br/> 
		</div> : <LabelSpinner></LabelSpinner>
		)
	);
}

export default PatternSettings;