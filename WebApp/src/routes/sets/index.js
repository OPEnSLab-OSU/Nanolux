import style from './style.css';
import Patterns from "../../components/patterns";
import NumericSlider from "../../components/numeric_slider";
import {useState, useEffect} from "preact/hooks";
import {
	getLoadedSubpattern,
	updateLoadedSubpattern,
	getLoadedPatternSettings,
	updateLoadedPattern,
	modifyLoadedSubpatternCount,
	getPatternList,
	getLoadedSubpatternCount,
	saveToSlot,
	loadSaveSlot} from "../../utils/api";
import {useConnectivity} from "../../context/online_context";
import useInterval from "../../utils/use_interval";

const Subpattern = ({subpattern, patterns}) => {

	const { isConnected } = useConnectivity();
	const [loading, setLoading] = useState(true);
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

	// Manage initalization
	useEffect(() => {
        if (isConnected) {
            getLoadedSubpattern(subpattern).then(data => {setData(data)}).then(setLoading(false));
        }  
    }, [isConnected])

	// Send out an update whenever something is changed
	useInterval(() => {
        if (isConnected && updated) {
            updateLoadedSubpattern(subpattern, data);
			setUpdated(false);
        }
    }, 100);

	// Update any parameters of the subpattern.
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

	// Generate the subpattern structure.
	return (
		(!loading ? 
		<div>
			<Patterns
				initialID={data.idx}
				structure_ref={"idx"}
				update={update}
				patterns={patterns}
			/>
			<NumericSlider
				className={style.settings_control}
				label="Brightness"
				min={0}
				max={255}
				initial={data.brightness}
				structure_ref="brightness"
				update={update}
			/>
			<NumericSlider
				className={style.settings_control}
				label="Smoothing"
				min={0}
				max={175}
				initial={data.smoothing}
				structure_ref="smoothing"
				update={update}
			/>
		</div> : <div></div>
		)
	);
}

const CurrentPattern = ({patterns}) => {

	const { isConnected } = useConnectivity();
	const [loading, setLoading] = useState(true);
	const [updated, setUpdated] = useState(false);

	// Subpattern data structure
	const [data, setData] = useState({
		subpattern_count: 1,
		alpha: 0,
		mode: 0,
		noise: 20
	});

	const [selectedSubpattern, setSubpattern] = useState(0);

	// Manage initalization
	useEffect(() => {
        if (isConnected) {
            getLoadedPatternSettings()
				.then(data => setData(data))
				.then(setLoading(false));
        }  
    }, [isConnected])

	// Send out an update whenever something is changed
	useInterval(() => {
        if (isConnected && updated) {
            updateLoadedPattern(data);
			setUpdated(false);
        }
    }, 100);

	// Update any parameters of the subpattern.
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

	const incrementSubpatterns = async () => {
		if(data.subpattern_count < 4){
			update("subpattern_count", data.subpattern_count + 1);
		}
	}

	const decrementSubpatterns = async () => {
		if(data.subpattern_count > 1){
			update("subpattern_count", data.subpattern_count - 1)
		}
	}

	function inRange(end){
		var l = [];
		for(var i = 0; i < end; i++){
			l.push({idx: i});
		}
		return l;
	}

	return (
		(!loading ? 
			<div>
				<NumericSlider
					className={style.settings_control}
					label="Transparency"
					min={0}
					max={255}
					initial={data.alpha}
					structure_ref="alpha"
					update={update}
				/>
				<NumericSlider
					className={style.settings_control}
					label="Mode"
					min={0}
					max={1}
					initial={data.mode}
					structure_ref="mode"
					update={update}
				/>
				<NumericSlider
					className={style.settings_control}
					label="Noise Threshold"
					min={0}
					max={100}
					initial={data.noise}
					structure_ref="noise"
					update={update}
				/>

				<button type="button" onClick={incrementSubpatterns}>+</button>
				<button type="button" onClick={decrementSubpatterns}>-</button>
				<br></br>

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
		: 'loading')
		
	);
}

const LoadButtons = (updateKey) => {

	const newKey = async (event) => {
		await loadSaveSlot(event.target.value);
		window.location.reload(true);
	}

	return(
		<div>
			<button type="button" onClick={newKey} value={0}>Load Default Save</button>
			<button type="button" onClick={newKey} value={1}>Load Save 1</button>
			<button type="button" onClick={newKey} value={2}>Load Save 2</button>
		</div>
	)

}

const SaveButtons = () => {

	const newKey = async (event) => {
		var success = await saveToSlot(event.target.value);
		if(!success){
			alert("Failed to save slot.");
		}
	}

	return(
		<div>
			<button type="button" onClick={newKey} value={0}>Save as Default</button>
			<button type="button" onClick={newKey} value={1}>Save as 1</button>
			<button type="button" onClick={newKey} value={2}>Save as 2</button>
		</div>
	)

}


const Settings = () => {

	const { isConnected } = useConnectivity();
	const [patterns, setPatterns] = useState([{index: 0, name: "None"}])

	useEffect(() => {
        if (isConnected) {
            getPatternList()
				.then(data => setPatterns(data))
				.then();
        }  
    }, [isConnected])
	

	const [key, updateKey] = useState(0);


	return (
		<div>
			<CurrentPattern
				patterns={patterns}
				key={key}
			/>
			<LoadButtons
				updateKey={updateKey}
			/>
			<SaveButtons/>
		</div>
	);
};

export default Settings;
