import style from './style.css';
import Patterns from "../../components/patterns";
import NumericSlider from "../../components/numeric_slider";
import {useState, useEffect} from "preact/hooks";
import {
	getLoadedSubpattern,
	updateLoadedSubpattern,
	getLoadedPatternSettings,
	updateLoadedPattern} from "../../utils/api";
import {useConnectivity} from "../../context/online_context";
import useInterval from "../../utils/use_interval";

const Subpattern = (subpattern, patternList) => {

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
		noise: 20
	});

	// Manage initalization
	useEffect(() => {
        if (isConnected) {
            getLoadedSubpattern(subpattern).then(data => setData(data));
            setLoading(false);
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
			var newData = data;
			newData[ref] = value;
			setData({data : newData});
			setUpdated(true);
		}
	}

	// Generate the subpattern structure.
	return (
		<div>
			<Patterns
				patternList={patternList}
				initialID={data.idx}
				structure_ref="idx"
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
		</div>
	);
}

const CurrentPattern = (initialSubpatternCount, patternList) => {

	const { isConnected } = useConnectivity();
	const [loading, setLoading] = useState(true);
	const [updated, setUpdated] = useState(false);

	// Subpattern data structure
	const [data, setData] = useState({
		subpattern_count: 1,
		alpha: 0,
		mode: 0
	});

	const [selectedSubpattern, setSubpattern] = useState(0);

	// Manage initalization
	useEffect(() => {
        if (isConnected) {
            getLoadedPatternSettings().then(data => setData(data));
            setLoading(false);
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
				const newData = Array.from(oldData);
				newData[ref] = value;
				return newData;
			})
		}
	}

	const incrementSubpatterns = () => {
		if(data.subpattern_count < 4){
			update("subpattern_count", data.subpattern_count + 1);
		}
	}

	const decrementSubpatterns = () => {
		if(data.subpattern_count > 1){
			update("subpattern_count", data.subpattern_count - 1);
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

			<button type="button" onClick={incrementSubpatterns}>+</button>
			<button type="button" onClick={decrementSubpatterns}>-</button> 

			{inRange(data.subpattern_count).map((data) => {
				return <button type="button" key={data.idx}>
					Subpattern {data.idx}
				</button>
			})}

			<Subpattern
				subpattern={selectedSubpattern}
				patternList={patternList}	
			/>
		</div>
	);
}

const Settings = () => {

	return (
		<div>
			<CurrentPattern
				initialSubpatternCount="1"
				patternList={{}}
			/>
		</div>
	);
};

export default Settings;
