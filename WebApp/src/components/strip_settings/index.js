import RANGE_CONSTANTS from '../../utils/constants';
import NumericSlider from "../numeric_slider";
import { useState, useEffect } from "preact/hooks";
import { getStripSettings, updateStripSettings } from "../../utils/api";
import { useConnectivity } from "../../context/online_context";
import useInterval from "../../utils/use_interval";
import { LabelSpinner } from '../spinner';
import SimpleChooser from '../single_chooser';
import style from './style.css';
import PatternSettings from '../pattern_settings';
import PatternModal from "../../components/pattern_modal";

/**
 * @brief A group of UI components for configuring LED strip settings on the NanoLux device.
 *
 * This component fetches initial strip settings from the device, allows users to
 * modify parameters (noise threshold, pattern count, mode, transparency), and
 * pushes updates back to the device.
 *
 * @param patterns   An array of pattern definitions passed down to PatternSettings.
 * @param advanced   A boolean indicating whether advanced settings should be displayed (default: false).
 *
 * @returns The group of StripSettings UI elements.
 */
const StripSettings = ({patterns, advanced = false}) => {
	// Controls whether the Pattern help modal is open.
	const [isModalOpen, setIsModalOpen] = useState(false);
	
	// Checks if the web app is connected to the device.
	const { isConnected } = useConnectivity();

	// Flag representing if initial data has been obtained from
	// the device.
	const [loading, setLoading] = useState(true);

	// Flag that tells the object to update the NanoLux device with new data.
	const [updated, setUpdated] = useState(false);

	// Stores the subpattern currently selected as an object state.
	const [selectedPattern, setPattern] = useState(0);

	// Strip-level data structure
	const [data, setData] = useState({
		pattern_count: 1,
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
            getStripSettings()
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
            updateStripSettings(data);
			setUpdated(false);
        }
    }, 100);

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
	 * @brief Increments the amount of patterns displayed. If the device is at
	 * maximum patterns, the function will refuse to increment.
	 */
	const incrementPatterns = async () => {
		if(data.pattern_count < RANGE_CONSTANTS.PATTERN_MAX){
			update("pattern_count", data.pattern_count + 1);
		}
	}

	/**
	 * @brief Decrements the amount of patterns displayed. If the device is at
	 * 1 pattern, the function will refuse to decrement. Moves to the
	 * previous pattern if the existing one is deleted.
	 */
	const decrementPatterns = async () => {
		if(data.pattern_count > 1){
			update("pattern_count", data.pattern_count - 1);
			if(data.pattern_count >= selectedPattern - 1){
				setPattern(data.pattern_count - 2);
			}	
		}
	}

	/** @brief Opens the pattern help modal. */
	const openModal = () => {
		setIsModalOpen(true);
	}

	/** @brief Closes the pattern help modal. */
	const closeModal = () => {
		setIsModalOpen(false);
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
				{advanced && (
					<>
					<br />
					<SimpleChooser
						label="Mode"
						tooltip={{
							id: 'mode',
							content: 'The mode that patterns will be displayed in.',
						}}
						options={[
							{option : "Strip Splitting", idx : RANGE_CONSTANTS.STRIP_SPLITTING_ID, tooltip : 'Splits the LED strip into sections up to 4 times based on how many patterns are added to the device.'},
							{option : "Z-Layering", idx : RANGE_CONSTANTS.Z_LAYERING_ID, tooltip : "Renders up to 2 patterns that are layered on top of each other over the entire LED strip."},
						]}
						noSelection={false}
						initial={data.mode}
						structure_ref="mode"
						update={update}
					/>
					<br />
					<NumericSlider
						className={style.settings_control}
						label="Transparency"
						tooltip={{
							id: 'transparency',
							content: 'Adjusts how opaque or transparent a pattern is.',
						}}
						min={RANGE_CONSTANTS.ALPHA_MIN}
						max={RANGE_CONSTANTS.ALPHA_MAX}
						initial={data.alpha}
						structure_ref="alpha"
						update={update}
					/>
					<br />
					</>
				)}
				<NumericSlider
					className={style.settings_control}
					label="Noise Threshold"
					tooltip={{
						id: 'threshold',
						content: 'Adjusts how much noise it takes to display a pattern.',
					}}
					min={RANGE_CONSTANTS.NOISE_MIN}
					max={RANGE_CONSTANTS.NOISE_MAX}
					initial={data.noise}
					structure_ref="noise"
					update={update}
				/>
				{advanced && (
					<>
					<br />
					<button className={style.incBtn} onClick={incrementPatterns} aria-label='Increase pattern count'>+</button>
					<button className={style.incBtn} onClick={decrementPatterns} aria-label='Decrease pattern count'>-</button>
					<br />
					<div className={style.patternRow}>
						{inRange(data.pattern_count).map((data) => {
							if(data.idx == selectedPattern){
								return <button className={style.patternBtn} onClick={function() {setPattern(data.idx);}} key={data.idx} style="border-style:inset;" aria-pressed='true'>
									Pattern {data.idx + 1}
								</button>
							}else{
								return <button className={style.patternBtn} onClick={function() {setPattern(data.idx);}} key={data.idx} aria-pressed='false'>
									Pattern {data.idx + 1}
								</button>
							}
						})}
					</div>
					</>
				)}
				<button
				  className={style.modalBtn}
				  type="button"
				  onClick={openModal}
				  aria-haspopup='dialog'
				  aria-label='Open help dialog for pattern descriptions'>
				  Help
				</button>
				<br />
				<hr />

				<PatternSettings
				  num={selectedPattern}
				  patterns={patterns}
				  advanced={advanced}
				  key={selectedPattern}
				/>
				<PatternModal
					isOpen={isModalOpen}
					onClose={closeModal}
				/>
			</div>
		: <LabelSpinner />)
	);
}

export default StripSettings;