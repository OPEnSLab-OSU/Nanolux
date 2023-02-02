import style from './style.css';
import Patterns from "../../components/patterns";
import NumericSlider from "../../components/numeric_slider";
import SimpleSlider from "../../components/simple_slider";

const Home = () => {
	return (
		<div className={style.home}>
			<Patterns />
			<NumericSlider />
			<NumericSlider />
			<SimpleSlider />
			<SimpleSlider />
		</div>
	);
};

export default Home;
