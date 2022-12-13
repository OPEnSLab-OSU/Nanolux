import { h } from 'preact';
import style from './style.css';


export default class Home {


  render() {
		return(
		<div class={style.home}>
			<h1>Home</h1>
			<h4 style="color:purple;">Audiolux: A Modern “Visual Sound” System for Deaf & Hard-of-Hearing</h4>
			<img src="https://www.cymaspace.org/wp-content/uploads/2015/11/Audiolux-One-System.jpg"  width="500" height="333"/>
			<p>AUDIOLUX is an open source digital lighting system that allows the Deaf & Hard-of-Hearing to see music & alerts using the Arduino hardware/software platform & modern digital individually addressable LEDs. Source code is provided under open-source licensing for anyone interested to develop their own system.
			</p>
		</div>
	)
  }
};
