import RANGE_CONSTANTS from '../../utils/constants';
import Subpattern from '../subpattern_settings';
import NumericSlider from "../../components/numeric_slider";
import {useState, useEffect} from "preact/hooks";
import {
	getLoadedPatternSettings,
	updateLoadedPattern} from "../../utils/api";
import {useConnectivity} from "../../context/online_context";
import useInterval from "../../utils/use_interval";
import { LabelSpinner } from '../../components/spinner';
import SimpleChooser from '../single_chooser';
import style from './style.css';

const CurrentPattern = ({patterns}) => {

	// Checks if the web app is connected to the device.
	const { isConnected } = useConnectivity();

	// Flag representing if initial data has been obtained from
	// the device.
	const [loading, setLoading] = useState(true);

	// Flag that tells the object to update the NanoLux device with new data.
	const [updated, setUpdated] = useState(false);

	// Stores the subpattern currently selected as an object state.
	const [selectedSubpattern, setSubpattern] = useState(0);

	// Pattern-level data structure
	const [data, setData] = useState({
		subpattern_count: 1,
		alpha: 0,
		mode: 0,
		noise: 20
	});

	/**
	 * @brief Manages initial querying of the data from the NanoLux device.
	 * 		  Sets the loading flag to false when done.
	 */
	useEffect(() => {
        if (isConnected) {
            getLoadedPatternSettings()
				.then(data => setData(data))
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
            updateLoadedPattern(data);
			setUpdated(false);
        }
    }, 100);

	/**
	 * @brief Updates a parameter in the subpattern data structure with a new value.
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
	 * @brief Increments the amount of subpatterns displayed. If the pattern is at
	 * maximum subpatterns, the function will refuse to increment.
	 */
	const incrementSubpatterns = async () => {
		if(data.subpattern_count < RANGE_CONSTANTS.SUBPATTERN_MAX){
			update("subpattern_count", data.subpattern_count + 1);
		}
	}

	/**
	 * @brief Decrements the amount of subpatterns displayed. If the pattern is at
	 * 1 subpattern, the function will refuse to decrement. Moves to the
	 * previous subpattern if the existing one is deleted.
	 */
	const decrementSubpatterns = async () => {
		if(data.subpattern_count > 1){
			if(data.subpattern_count == selectedSubpattern + 1){
				setSubpattern(data.subpattern_count - 1);
			}
			update("subpattern_count", data.subpattern_count - 1);
		}
	}

	/**
	 * @brief Generates a list from 0 to end. Analagous to Python's
	 * range() function.
	 * @param end The value to end at.
	 */
	function inRange(end){
		var l = [];
		for(var i = 0; i < end; i++){
			l.push({idx: i});
		}
		return l;
	}

	/**
	 * @brief Generates the Pattern UI element and it's selected subpattern.
	 */
	return (
		(!loading ? 
			<div>
				<SimpleChooser
					label="Mode"
					options={[
						{option : "Strip Splitting", idx : RANGE_CONSTANTS.STRIP_SPLITTING_ID},
						{option : "Z-Layering", idx : RANGE_CONSTANTS.Z_LAYERING_ID},
					]}
					noSelection={false}
					initial={data.mode}
					structure_ref="mode"
					update={update}
				/>
				<br/>
				<NumericSlider
					className={style.settings_control}
					label="Transparency"
					min={RANGE_CONSTANTS.ALPHA_MIN}
					max={RANGE_CONSTANTS.ALPHA_MAX}
					initial={data.alpha}
					structure_ref="alpha"
					update={update}
				/>
				<br/>
				<NumericSlider
					className={style.settings_control}
					label="Noise Threshold"
					min={RANGE_CONSTANTS.NOISE_MIN}
					max={RANGE_CONSTANTS.NOISE_MAX}
					initial={data.noise}
					structure_ref="noise"
					update={update}
				/>
				<br/>
				<button type="button" onClick={incrementSubpatterns}>+</button>
				<button type="button" onClick={decrementSubpatterns}>-</button>
				<hr></hr>

				{inRange(data.subpattern_count).map((data) => {
					if(data.idx == selectedSubpattern){
						return <button type="button" onClick={function() {setSubpattern(data.idx);}} key={data.idx} style="border-style:inset;">
							SP {data.idx}
						</button>
					}else{
						return <button type="button" onClick={function() {setSubpattern(data.idx);}} key={data.idx}>
							SP {data.idx}
						</button>
					}
					
				})}

				<Subpattern
					subpattern={selectedSubpattern}
					patterns={patterns}
					key={selectedSubpattern}
				/>	
			</div> 
		: <LabelSpinner></LabelSpinner>)
		
	);
}

export default CurrentPattern;