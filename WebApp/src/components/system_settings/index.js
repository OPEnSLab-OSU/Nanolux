import NumericSlider from "../../components/numeric_slider";
import {useState, useEffect} from "preact/hooks";
import {
	updateDeviceSettings,
	getDeviceSettings} from "../../utils/api";
import {useConnectivity} from "../../context/online_context";
import useInterval from "../../utils/use_interval";
import SimpleChooser from '../../components/single_chooser';
import { LabelSpinner } from '../../components/spinner';
import RANGE_CONSTANTS from '../../utils/constants';
import style from './style.css';

/**
 * @brief Generates a UI element for changing NanoLux device system settings.
 */
const SystemControls = () => {

	// Checks if the web app is connected to the device.
	const { isConnected } = useConnectivity();

	// Flag representing if initial data has been obtained from
	// the device.
	const [loading, setLoading] = useState(true);

	// Flag that tells the object to update the NanoLux device with new data.
	const [updated, setUpdated] = useState(false);

	// System settings data structure
	const [data, setData] = useState({
		length: 60,
		loop: 40,
		debug: 0
	});

	/**
	 * @brief Manages initial querying of the data from the NanoLux device.
	 * 		  Sets the loading flag to false when done.
	 */
	useEffect(() => {
		if (isConnected) {
			getDeviceSettings()
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
			updateDeviceSettings(data);
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
	 * @brief Generates the UI element for adjusting system settings.
	 */
	return (
		(!loading ? 
			<div>
				<NumericSlider
					className={style.settings_control}
					label="LED Strip Length"
					tooltip={{
						id: 'length',
						content: 'Adjusts how much of the LED strip is used by the device to display patterns',
					}}
					min={RANGE_CONSTANTS.LENGTH_MIN}
					max={RANGE_CONSTANTS.LENGTH_MAX}
					initial={data.length}
					structure_ref="length"
					update={update}
				/>
				<br />
				<NumericSlider
					className={style.settings_control}
					label="LED Update Time (ms)"
					tooltip={{
						id: 'update',
						content: 'Adjusts how fast patterns are updated on the LED strip. Lower values mean patterns are updated faster.',
					}}
					min={RANGE_CONSTANTS.LOOP_MIN}
					max={RANGE_CONSTANTS.LOOP_MAX}
					initial={data.loop}
					structure_ref="loop"
					update={update}
				/>
				<br />
				<SimpleChooser
					className={style.settings_control}
					label="Debug Mode"
					tooltip={{
						id: 'debug',
						content: 'Optional debugging modes for development.',
					}}
					options={[
						{option : "Debug Out", idx : 1, tooltip : "Prints pattern and strip data to the serial console for debugging."},
						{option : "Simulator Out", idx : 2, tooltip : "Prints full LED frames as comma-separated (R,G,B) values to the serial console for simulating patterns externally."},
					]}
					noSelection={true}
					initial={data.debug}
					structure_ref="debug"
					update={update}	
				/>
				<br />
			</div> 
		: <LabelSpinner />)		
	);
}

export default SystemControls;