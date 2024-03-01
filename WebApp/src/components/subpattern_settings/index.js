import Patterns from "../../components/patterns";
import NumericSlider from "../../components/numeric_slider";
import {useState, useEffect} from "preact/hooks";
import {
	getLoadedSubpattern,
	updateLoadedSubpattern} from "../../utils/api";
import {useConnectivity} from "../../context/online_context";
import useInterval from "../../utils/use_interval";
import { LabelSpinner } from '../../components/spinner';
import RANGE_CONSTANTS from '../../utils/constants';
import style from './style.css';

/**
 * @brief An object meant to hold and display settings for a specific subpattern.
 * @param subpattern The ID of the subpattern to display
 * @param patterns	 A list of patterns and their IDs
 */
const Subpattern = ({subpattern, patterns}) => {

	// Checks if the web app is connected to the device.
	const { isConnected } = useConnectivity();

	// Flag representing if initial data has been obtained from
	// the device.
	const [loading, setLoading] = useState(true);

	// Flag that tells the object to update the NanoLux device with new data.
	const [updated, setUpdated] = useState(false);

	// Subpattern data structure
	const [data, setData] = useState({
		idx: 0,
		hue_max: 0,
		hue_min: 255,
		brightness: 255,
		smoothing: 0,
		direction: 0,
	});

	/**
	 * @brief Manages initial querying of the data from the NanoLux device.
	 * 		  Sets the loading flag to false when done.
	 */
	useEffect(() => {
        if (isConnected) {
            getLoadedSubpattern(subpattern)
			.then(data => {setData(data)})
			.then(setLoading(false));
        }  
    }, [isConnected])

	/**
	 * @brief Updates the subpattern on the Nanolux device 
	 * if it is connected and has modified data,
	 * then flags the data structure.
	 */
	useInterval(() => {
        if (isConnected && updated) {
            updateLoadedSubpattern(subpattern, data);
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
			<NumericSlider
				className={style.settings_control}
				label="Brightness"
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
				min={RANGE_CONSTANTS.SMOOTHING_MIN}
				max={RANGE_CONSTANTS.SMOOTHING_MAX}
				initial={data.smoothing}
				structure_ref="smoothing"
				update={update}
			/>
		</div> : <LabelSpinner></LabelSpinner>
		)
	);
}

export default Subpattern;